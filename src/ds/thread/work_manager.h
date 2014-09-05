#pragma once
#ifndef DS_THREAD_WORKMANAGER_H_
#define DS_THREAD_WORKMANAGER_H_

#include <string>
#include <vector>
#include <memory>
#include <Poco/ThreadPool.h>
#include "ds/thread/thread_defs.h"
#include "ds/thread/work_request.h"

namespace ds {
class WorkClient;

/**
 * \class ds::WorkManager
 * \brief Run a thread pool that can be continually fed WorRequests. These requests are generally
 * mediated through a WorkClient subclass, which handles the broad types of requests an app might
 * want.  Typically, the app will instantiate a WorkClient and let it take care of all the details.
 */
class WorkManager
{
public:
	WorkManager();
	~WorkManager();

	// I take ownership of the request.
	bool							sendRequest(std::unique_ptr<WorkRequest>&, Poco::Timestamp* sendTime = nullptr);

	// Called from the world engine during each update cycle, which is probably
	// excessive, but the performance hit is nil.  This is where we handle
	// any pending query outputs.
	void							update();

	// Stop the thread pool.  Called from the destructor, if a client doesn't call it earlier.
	void							stopManager();

protected:
	friend class WorkClient;

	void							addClient(WorkClient&);
	void							removeClient(WorkClient&);

	// Thread entry
private:
	class Loop : public Poco::Runnable {
	public:
		Loop(WorkManager&);

		// needs to be reentrant, since we are passed to multiple threads
		virtual void				run();

		void						handleInput(std::unique_ptr<WorkRequest>&) const;

	private:
		WorkManager&				mManager;
	};

private:
	typedef std::vector<std::unique_ptr<WorkRequest>> RequestList;

	/* NOTE ON LOCK ORDER:  Clients and the output can have nested locks.  Client
	 * is always locked first.  XXX actually I think that changed.  I think there's
	 * no nesting at the moment.
	 */
	// Synchronization
	Poco::ThreadPool				mPool;
	Loop							mLoop;

	// Input
	Poco::Mutex						mInputMutex;
	RequestList						mInput;

	// Output
	Poco::Mutex						mOutputMutex;
	RequestList						mOutput, mOutputTmp;

	// Clients
	Poco::Mutex						mClientMutex;
	std::vector<WorkClient*>		mClient;

	// Call after you've added more input
	bool							inputAdded();

	// Find the best match with database and put it in the list
	void							popNextInput(RequestList&);

	// Add to the output list
	void							addOutput(std::unique_ptr<WorkRequest>&);

	// Answer the client, if it exists.  Assumes the client list is locked.
	WorkClient*						findClientLocked(const void* clientId);

public:
	class InputFactory;
	class OutputFactory;

#ifdef DS_THREAD_DEBUG_IS_ON
private:
	QueryDebug::Statistics			mStatistics;	// use input mutex
	void							debugThreadStarted(void*);
	void							debugThreadStopped(void*);
public:
	void							debugGetStatistics(QueryDebug::Statistics&);
#endif
};

} // namespace ds

#endif // DS_THREAD_WORKMANAGER_H_