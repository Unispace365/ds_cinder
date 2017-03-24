#include "stdafx.h"

#include "ds/thread/work_manager.h"

#include <algorithm>
#include <iostream>
#include "ds/thread/work_client.h"

using namespace ds;
//using namespace std;

static const std::string					WORK_THREAD_NAME("ds_work");

/**
 * \class ds::WorkManager
 */
WorkManager::WorkManager()
	: mPool(WORK_THREAD_NAME, 4, 16)		// Keep at least 4 threads running, because we use this for all async ops
//	: mPool(WORK_THREAD_NAME, 1, 1)
	, mLoop(*this)
{
	mClient.reserve(64);
	mInput.reserve(64);
	mOutput.reserve(64);
	mOutputTmp.reserve(64);
}

WorkManager::~WorkManager()
{
	stopManager();
}

void WorkManager::addClient(WorkClient& c)
{
	Poco::Mutex::ScopedLock		l(mInputMutex);
	try {
		mClient.push_back(&c);
	} catch (std::exception const&) {
	}
}

void WorkManager::removeClient(WorkClient& c)
{
	Poco::Mutex::ScopedLock		l(mInputMutex);
	try {
		mClient.erase( remove( mClient.begin(), mClient.end(), &c ), mClient.end() );
	} catch (std::exception const&) {
	}
}

bool WorkManager::sendRequest(std::unique_ptr<WorkRequest> upR, Poco::Timestamp* sendTime)
{
	if (!upR.get()) return false;
	// Push new input onto the stack
	{
		Poco::Mutex::ScopedLock		l(mInputMutex);
		try {
			upR.get()->mRequestTime = Poco::Timestamp();
			if (sendTime) *sendTime = upR.get()->mRequestTime;
			mInput.push_back(std::move(upR));
		} catch (std::exception&) {
			return false;
		}
	}
	return inputAdded();
}

void WorkManager::stopManager()
{
	// Clear out the inputs so the threads will finish.
	{
		Poco::Mutex::ScopedLock		l(mInputMutex);
		mInput.clear();
	}

	try {
		mPool.joinAll();
	} catch (std::exception&) {
	}
}

void WorkManager::update()
{
	// To control how much processing the client does, pop off a single result
	// in an update cycle.
	mOutputTmp.clear();
	{
		Poco::Mutex::ScopedLock		l(mOutputMutex);
		if (!mOutput.empty()) {
			// I suspect there's a 1-line way to do this but haven't found it so far
//			std::move(mOutput.begin(), mOutput.begin()+1, mOutputTmp.end());
			std::unique_ptr<WorkRequest>	r(std::move(mOutput.front()));
			mOutput.erase(mOutput.begin());
			mOutputTmp.push_back(std::move(r));
		}
	}
	if (mOutputTmp.empty()) return;

	{
		Poco::Mutex::ScopedLock		l(mClientMutex);
		for (auto it=mOutputTmp.begin(), end=mOutputTmp.end(); it != end; ++it) {
			WorkRequest*			r(it->get());
			if (!r) continue;
			WorkClient*				client = findClientLocked(r->mClientId);
			if (!client) continue;

			client->handleResult(*it);
		}
	}
	// Any requests that weren't claimed by a client are lost
	mOutputTmp.clear();
}

bool WorkManager::inputAdded()
{
	// Start a new thread to handle the input.  If we can't start one, no big deal,
	// that means we've got threads running that will handle it.  And note that
	// we always have a minimum number of threads running, so there's always someone
	// who can handle it eventually.
	try {
		// This is a little ugly, but I have VS set to break on handled exceptions, which I want
		// in general.  However, it's pretty easy to use up my thread pool, and I don't care
		// about those warnings.  BUT, in practice, I imagine there's a remote possibility that
		// somewhere after checking if threads are available but before calling start(), all the
		// threads might HAVE been in use but time out, causing a thread not to get started.
		// Granted, even if this happens it just means an input will sit there until someone
		// else starts a thread, but still, in practice, I always want to try and create a thread.
#ifdef _DEBUG
		if (mPool.available() > 1) {
			mPool.startWithPriority(Poco::Thread::PRIO_LOW, mLoop);
		}
#else
		mPool.startWithPriority(Poco::Thread::PRIO_LOW, mLoop);
#endif
	} catch (Poco::NoThreadAvailableException&) {
	} catch (std::exception&) {
	}

	return true;
}

void WorkManager::popNextInput(RequestList& in)
{
	Poco::Mutex::ScopedLock		l(mInputMutex);
	if (!mInput.empty()) {
		if (in.empty()) {
			in.swap(mInput);
		} else {
			std::move(mInput.begin(), mInput.end(), in.begin());
		}
	}
}

void WorkManager::addOutput(std::unique_ptr<WorkRequest>& r)
{
	if (!r.get()) return;
	Poco::Mutex::ScopedLock		l(mOutputMutex);
	try {
		mOutput.push_back(std::move(r));
	} catch (std::exception const&) {
	}
}

WorkClient* WorkManager::findClientLocked(const void* clientId)
{
	auto it = std::find(mClient.begin(), mClient.end(), (WorkClient*)clientId);
	if (it == mClient.end()) return nullptr;
	return *it;
}

/**
 * \class ds::WorkManager::Loop
 */
WorkManager::Loop::Loop(WorkManager& qm)
	: mManager(qm)
{
}

void WorkManager::Loop::run()
{
	DS_DBG_THREAD_CODE(mManager.debugThreadStarted(Poco::Thread::current()));

	// Run for as long as I have input, then let the thread die to be reclaimed.
	RequestList						ins;
	mManager.popNextInput(ins);
	while (!ins.empty()) {
		// Process
		for (auto it=ins.begin(), end=ins.end(); it != end; ++it) {
			handleInput(*it);
		}
		ins.clear();

		// Testing!  Which is not great -- should have some randomization
		// so they're not all in sync.
//		Poco::Thread::sleep(500);

		// Continue processing if we've got input
		mManager.popNextInput(ins);
	}

	DS_DBG_THREAD_CODE(mManager.debugThreadStopped(Poco::Thread::current()));
}

void WorkManager::Loop::handleInput(std::unique_ptr<WorkRequest>& upR) const
{
	WorkRequest*			r = upR.get();
	if (!r) return;

	r->run();

  mManager.addOutput(upR);
}

/* QUERY-DEBUG
 ******************************************************************/
#if QUERY_DEBUG_IS_ON

void QueryManager::debugThreadStarted(void* id)
{
	Poco::Mutex::ScopedLock		l(mInputMutex);
	mStatistics.mThreadId.add(id);
}

void QueryManager::debugThreadStopped(void* id)
{
	Poco::Mutex::ScopedLock		l(mInputMutex);
	mStatistics.mThreadId.remove(id);
}

void QueryManager::debugGetStatistics(QueryDebug::Statistics& s)
{
	{
		Poco::Mutex::ScopedLock		l(mInputMutex);
		s.mInputSize = mInput.size();
		s.mThreadId = mStatistics.mThreadId;
	}
	{
		Poco::Mutex::ScopedLock		l(mInputMutex);
		s.mOutputSize = mOutput.size();
	}
	s.mUsed = mPool.used();
	s.mAllocated = mPool.allocated();
}

#endif
