#include "stdafx.h"

#include "ds/thread/gl_thread.h"

#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include <Poco/Semaphore.h>

using namespace ds;

namespace {
const ds::BitMask GLTHREAD_LOG_M = ds::Logger::newModule("gl_thread");

class WaitCallback : public GlThreadCallback {
  public:
	Poco::Semaphore mSem;

	WaitCallback()
	  : mSem(0, 1) {}
	~WaitCallback() {}
	virtual void consume(const bool) { mSem.set(); }
	virtual bool matches(const GlThreadCallback*) const { return false; }
};

} // namespace

/* DS::GL-THREAD
 ******************************************************************/
GlThread::GlThread() {}

void GlThread::start(const bool makeGlCalls) {
	if (mThread.isRunning()) {
		// DS_LOG_WARNING_M("ds::GlThread::start() thread already running", GLTHREAD_LOG_M);
		return;
	}

	mLoop.start(makeGlCalls);
	if (!mLoop.mError) {
		mThread.start(mLoop);
		// We want to serialize this thread with the main thread
		if (makeGlCalls && mThread.isRunning()) waitForNoInput();
	}
}

GlThread::~GlThread() {
	if (mLoop.mError) return;

	{
		Poco::Mutex::ScopedLock l(mLoop.mMutex);
		mLoop.mAbort = true;
		mLoop.mCondition.signal();
	}

	try {
		mThread.join();
	} catch (std::exception const&) {}
}

bool GlThread::performOnWorkerThread(GlThreadCallback* cb) {
	if (!cb) return false;
	// If the thread isn't running, prevent items from being endlessly
	// pushed on the stack, and also give the callback a chance to memory manage itself.
	if (!mThread.isRunning()) {
		DS_LOG_WARNING_M(
			"ds::GlThread::performOnWorkerThread() thread is not running, operation consumed without executing",
			GLTHREAD_LOG_M);
		cb->consume(false);
		return false;
	}

	return mLoop.addInput(cb);
}

void GlThread::waitForNoInput() {
	// Pass a new object through the system and then block until it finishes
	WaitCallback wd;
	performOnWorkerThread(&wd);
	wd.mSem.wait();
}

/* DS::GL-THREAD::LOOP
 ******************************************************************/
GlThread::Loop::Loop()
  : mAbort(false)
  , mError(true) {}

GlThread::Loop::~Loop() {
	// No need to do anything with the background context.
	// Destructors will take are of things automatically.
}

bool GlThread::Loop::start(const bool makeGlCalls) {
	if (makeGlCalls) {
		DS_REPORT_GL_ERRORS();
		mBackgroundContext = ci::gl::Context::create(ci::gl::context());
		DS_REPORT_GL_ERRORS();
		if (mBackgroundContext) mError = false;
	} else {
		mError = false;
	}
	return !mError;
}

bool GlThread::Loop::makesGlCalls() const {
	return (mBackgroundContext != nullptr);
}

bool GlThread::Loop::addInput(GlThreadCallback* cb) {
	if (!cb) return false;

	Poco::Mutex::ScopedLock l(mMutex);
	// If aborting, no reason to go further
	if (mAbort) {
		cb->consume(false);
		return false;
	}

	try {
		mInput.push_back(cb);
	} catch (std::exception const&) {
		return false;
	}

	mCondition.signal();
	return true;
}

void GlThread::Loop::run() {
	// Set the GL context, if this is actually a GL thread
	const bool glCalls = makesGlCalls();
	if (glCalls) {
		// When setting the GL context you need to make sure this call
		// is synchronized with the main thread, or else weird things can happen.
		if (mBackgroundContext) mBackgroundContext->makeCurrent();
		DS_REPORT_GL_ERRORS();
	}

	// I want to stay locked for as little time as possible, so I pop off
	// the active inputs, then pop them back on as retired for reuse.
	std::vector<GlThreadCallback*> ins;
	ins.reserve(16);

	while (true) {
		// Pop the inputs
		{
			Poco::Mutex::ScopedLock l(mMutex);
			mInput.swap(ins);
		}

		// Perform each input
		consume(ins);

		// Before stopping the thread make sure to clear out everyone
		// in the input list, since clients can place objects to be deleted there
		bool aborting = false;
		{
			Poco::Mutex::ScopedLock l(mMutex);
			if (mAbort) {
				aborting = true;
				mInput.swap(ins);
			}
		}
		if (aborting) {
			consume(ins);
			break;
		}

		// If more input came in during the time I've been
		// processing keep going, otherwise wait.
		mMutex.lock();
		if (!mAbort && mInput.size() < 1) mCondition.wait(mMutex);
		mMutex.unlock();
	}

	if (glCalls) {
		// Destroy the background context
		if (mBackgroundContext) mBackgroundContext.reset();
	}
}

void GlThread::Loop::consume(std::vector<GlThreadCallback*>& ins) {
	// Perform each callback.  There's one special case:  Callbacks of matching runs
	// are considered a batch, and only the final one will be performed.
	const int		  size = static_cast<int>(ins.size());
	GlThreadCallback *cur, *nxt;
	for (int k = 0; k < size; k++) {
		if ((cur = ins[k]) == NULL) continue;
		// If I match the next item, consume me without running me, otherwise consume and run.
		// Either way, we always have to consume, so the callback can do any necessary memory management.
		const bool run = (k + 1 >= size || (nxt = ins[k + 1]) == NULL || !(cur->matches(nxt)));
		cur->consume(run);
	}
	ins.clear();
}

/* DS::GL-NO-THREAD
 ******************************************************************/
GlNoThread::GlNoThread() {
	mLoop.mError = true;
}

void GlNoThread::start(const bool) {
	DS_LOG_ERROR_M("GlNoThread::start() should not be starting NoThread", GLTHREAD_LOG_M);
}

bool GlNoThread::performOnWorkerThread(GlThreadCallback* cb) {
	DS_LOG_ERROR_M("GlNoThread::performOnWorkerThread() should not be sending operations to NoThread", GLTHREAD_LOG_M);
	return GlThread::performOnWorkerThread(cb);
}

void GlNoThread::waitForNoInput() {
	DS_LOG_ERROR_M("GlNoThread::waitForNoInput() should not be sending operations to NoThread", GLTHREAD_LOG_M);
}
