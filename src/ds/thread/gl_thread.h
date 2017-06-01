#pragma once
#ifndef DS_THREAD_GLTHREAD_H_
#define DS_THREAD_GLTHREAD_H_

#include <algorithm>
#include <assert.h>
#include <vector>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#ifdef _DEBUG
#include <Poco/Debugger.h>
#endif

#include <cinder/gl/Context.h>

/* DS::GL-THREAD
 * Utility for any clients that want to render to a GL texture in
 * a separate thread.  There are two pieces to be aware of:
 * 
 * The GlThreadClient is subclassed by anyone that needs a worker
 * thread.  It has a single function for performing operations on
 * that thread.  Note that you can also pass off a delete function
 * to the worker thread for the GlThreadClient subclass, if the
 * class has state that needs to be cleaned up in the worker thread.
 *
 * The GlThread is owned by an object outside of the thread client,
 * and must exist for the life of the thread client.
 *
 * FLOW OF CONTROL
 * ** MAIN THREAD **
 * ThreadClient requests perform
 * - callback is pushed onto thread input queue
 * - input queue is signaled to wake up
 * - return
 * ** WORKER THREAD **
 * Worker is woken up
 * - removes all inputs from the queue
 * - processes each input
 * - goes back to sleep
 ******************************************************************/

// If this is on then record every entry and exit from each operation run in the thread
//#define THREAD_DS_LOG(stream)	DS_LOG(stream)
#define THREAD_DS_LOG(stream)		;

namespace ds {
class GlThread;

/* DS::GL-THREAD-CALLBACK
 * This is essentially an internal class.
 ******************************************************************/
class GlThreadCallback {
	// Subclasses are completely responsible for managing their memory,
	// no outside API will do that for them.
	protected:	virtual ~GlThreadCallback()			{ }
	// The consume calls the operation which could be anything, including
	// something that results in the deletion of this object.  Once consume
	// is called, the object must be considered invalid.  If run is true,
	// then the operation should be performed, otherwise it's being batched
	// up with identical operations.
	public:		virtual void consume(const bool run) = 0;
	// Match is used to batch up operations.  Runs of matching neighbors
	// will only have the final callback run.
	public:		virtual bool matches(const GlThreadCallback*) const = 0;
};

/* DS::GL-THREAD-CLIENT
 * Anyone interested in running async operations on a GLThread
 * needs to subclass this client.
 ******************************************************************/
template <class T>
class GlThreadClient {
public:
	GlThreadClient(GlThread&);
	virtual ~GlThreadClient();

	// Start a remote operation.  Note that it's safe for the subclass
	// to call a method that will result in deleting the subclass, since
	// all operations are serialized.  If batch is true, then any neighbor
	// operations with the same class and method are combined into one.
	bool							performOnWorkerThread(void(T::*callerMethod)(), const bool batch = false);
	// Block until all worker thread operations have finished.  Note there
	// is no guarantee that operations won't be started after you've
	// requested the wait, this just waits until whatever inputs that are
	// currently present at the time waitForNoInput() is called to finish.
	void							waitForNoInput();

private:
	class ClientCallback : public GlThreadCallback {
		public:
			ClientCallback(GlThreadClient& tc, T *caller) : mOwner(tc) {
				mSavedClass = caller;
				mSavedMethod = NULL;
				mBatch = false;
			}

			void setup(void(T::*callerMethod)(), const bool batch)	{ mBatch = batch; mSavedMethod = callerMethod; }
			void consume(const bool run) {
				// Grab my class and method info so I can place myself back into the
				// retired list before running my operation.  I need to do this because
				// the operation could be anything, including deleting my owner.
				T *localClass = mSavedClass;
				void (T::*localMethod)() = mSavedMethod;
				// Push me onto my owner for reuse
				mOwner.endCallback(*this);
				// Run my operation, which might end up deleting me.
				if (run && localClass && localMethod )
				{
					THREAD_DS_LOG("begin thread class=" << typeid(T).name() << " method=" << &(mSavedMethod) << " this=" << ((void*)this) << endl);
					(*localClass.*localMethod)();
					THREAD_DS_LOG("\tend thread class=" << typeid(T).name() << " method=" << &(mSavedMethod) << " this=" << ((void*)this) << endl);
				}
			}
			bool matches(const GlThreadCallback* cb) const {
				if (!mBatch) return false;
				const ClientCallback*			o = dynamic_cast<const ClientCallback*>(cb);
				if (!o) return false;
				return o->mBatch && mSavedClass == o->mSavedClass && mSavedMethod == o->mSavedMethod;
			}

		private:
			GlThreadClient&	mOwner;
			bool			mBatch;
			T				*mSavedClass;
			void (T::*mSavedMethod)();
	};

private:
	GlThread&						mT;
	Poco::Mutex						mMutex;
	// This class is responsible for completely managing memory for any callbacks it creates.
	// They are never owned by anyone else.
	std::vector<ClientCallback*>	mActive, mRetired;

	GlThreadCallback*				startCallback(void(T::*callerMethod)(), const bool batch);
	void							endCallback(ClientCallback&);
};

/* DS::GL-THREAD-CLIENT impl
 ******************************************************************/
template <class T>
GlThreadClient<T>::GlThreadClient(GlThread& t)
	: mT(t)
{
	mActive.reserve(16);
	mRetired.reserve(16);
}

template <class T>
GlThreadClient<T>::~GlThreadClient()
{
#ifdef _DEBUG
	if (mActive.size() > 0) {
		// This is a serious error.  It means the thread is deleting, and taking any
		// operations with it, but there are still operations to be performed.
		Poco::Debugger::enter();
	}
#endif
	for (int k=0; k<mActive.size(); k++) delete mActive[k];
	for (int k=0; k<mRetired.size(); k++) delete mRetired[k];
}



template <class T>
GlThreadCallback* GlThreadClient<T>::startCallback(void(T::*callerMethod)(), const bool batch)
{
	// Pop off the next available callback or allocate a new one
	ClientCallback*				ans = NULL;
	Poco::Mutex::ScopedLock		l(mMutex);
	if (mRetired.size() > 0) {
		ans = mRetired.back();
		mRetired.pop_back();
	}
	if (!ans && !(ans = new ClientCallback(*this, (T *)this))) return NULL;
	ans->setup(callerMethod, batch);
	try {
		mActive.push_back(ans);
	} catch(std::exception const&) {
	}
	THREAD_DS_LOG("Perform on worker tc=" << getTcId() << " cb=" << ans->getId() << endl);
	return ans;
}

template <class T>
void GlThreadClient<T>::endCallback(ClientCallback& cb)
{
	try {
		Poco::Mutex::ScopedLock					l(mMutex);
		mActive.erase( remove( mActive.begin(), mActive.end(), &cb ), mActive.end() );
		mRetired.push_back(&cb);
	} catch (std::exception const&) {
		// This shouldn't ever happen.
		assert(false);
		delete &cb;
	}
}

/* DS::GL-THREAD
 * Create a worker thread for clients to push operations onto.
 * The main thread creates an instance of this class, and then
 * anyone that wants to utilize it creates a client that can
 * start and manage operations.
 ******************************************************************/
class GlThread {
public:
	GlThread();
	virtual ~GlThread();

	// Start needs to be called sometime after the app setup, where
	// GL gets initialized.
	virtual void				start(const bool makeGlCalls);

	virtual bool				performOnWorkerThread(GlThreadCallback*);
	// Block until all worker thread operations have finished.  Note there
	// is no guarantee that operations won't be started after you've
	// requested the wait, this just waits until whatever inputs that are
	// currently present at the time waitForNoInput() is called to finish.
	virtual void				waitForNoInput();

	// LOOP - thread entry
private:
	class Loop : public Poco::Runnable {
	public:
		Poco::Mutex				mMutex;
		Poco::Condition			mCondition;
		bool			    	mAbort;
		ci::gl::ContextRef		mBackgroundContext;
		bool			    	mError;

	public:
		Loop();
		~Loop();

		bool					start(const bool makeGlCalls);
		bool			    	makesGlCalls() const;
		bool					addInput(GlThreadCallback*);
		virtual void			run();

	private:
		std::vector<GlThreadCallback*> mInput;

		void				    consume(std::vector<GlThreadCallback*>&);
	};

protected:
	Loop						mLoop;

private:
	GlThread(const GlThread&);
	GlThread&				  	operator=(const GlThread&);

	Poco::Thread				mThread;
};

/* DS::GL-NO-THREAD
 * A special GlThread subclass that never starts or runs a thread.
 * Used so that I can still have a thread placeholder class in the
 * engine when necessary, but no overhead is incurred.
 ******************************************************************/
class GlNoThread : public GlThread {
  public:
    GlNoThread();

    virtual void        start(const bool makeGlCalls);
    virtual bool				performOnWorkerThread(GlThreadCallback*);
    virtual void				waitForNoInput();
};

template <class T>
bool GlThreadClient<T>::performOnWorkerThread(void(T::*callerMethod)(), const bool batch)
{
	return mT.performOnWorkerThread(startCallback(callerMethod, batch));
}

template <class T>
void GlThreadClient<T>::waitForNoInput()
{
	mT.waitForNoInput();
}

} // namespace ds

#endif // DS_THREAD_GLTHREAD_H_
