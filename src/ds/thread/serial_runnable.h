#pragma once
#ifndef DS_THREAD_SERIALRUNNABLE_H_
#define DS_THREAD_SERIALRUNNABLE_H_

#include <functional>
#include <vector>
#include "ds/thread/runnable_client.h"

namespace ds {

/* DS::SERIAL-RUNNABLE
 * A utility for running a single async operation at a time.  Clients
 * define the type through the template, set a function to handle any
 * replies, then kick of a query with start().
 *
 * NOTE: Throws if the runnable can't be created
 *
 * This class favours CPU efficiency over performance: Only one op
 * can be running at a time. When start() is called before one finishes,
 * then an op will buffer up and begin as soon as the current is done.
 * The downside to this is for long operations, you will have to wait
 * up to twice as long for the final result. The upside is that clients
 * can completely ignore performance and start() new operations as
 * frequently as they need without drowning the system. For clients that
 * really need an op to start immediately, they should probably include
 * a way to stop the running operation.
 *
 *** IMPLICIT INTERFACE
 *
 * T is a subclass of Poco::Runnable
 ******************************************************************/
template <class T>
class SerialRunnable {
public:
	typedef std::function<void (T&)>	HandlerFunc;

public:
  // If T has a constructor without arguments, ignore the alloc. If you need
  // to supply info to the constructor, supply a custom alloc.
	SerialRunnable(ui::SpriteEngine&, const std::function<T*(void)>& alloc = nullptr);
	
	void							  setReplyHandler(const HandlerFunc& f) { mReplyHandler = f; }
	// Start a new runnable, intializing it via the handler block.  Any previous, unfinished runs
	// will be ignored.
  // If waitForResult is true, this will be run synchronously, blocking until the operation is finished.
  // This is useful for serial runnables that run through the life of an app, but during setup you
  // want to run them once and guarantee data exists before the app begins.
  // NOTE: Be careful with HandlerFunc; it might be evaluated at some later point. So if you
  // pass in a lambda with captured variables, make sure to capture by value, not reference, or if
  // you do, that the referenced item still exists for the life of the SerialRunnable.
	bool							  start(const HandlerFunc& = nullptr, const bool waitForResult = false);

private:
	bool							  send();
	void								receive(std::unique_ptr<Poco::Runnable>&);

  RunnableClient      mClient;
  enum State {
      CACHED,         // mCache is valid, nothing is running --NOTE this state is redundant with mCache existing, so favour that
      RUNNING,        // mCache is empty, and as soon as I receive something, I'm done
      WAITING         // mCache is empty, as soon as I receive something, I start again
  };
  State               mState;
  std::unique_ptr<T>  mCache;
	HandlerFunc				  mReplyHandler;
  // If start comes in and I can't start yet, cache the start handler
  HandlerFunc         mStartHandler;
};

template <class T>
SerialRunnable<T>::SerialRunnable(ui::SpriteEngine& se, const std::function<T*(void)>& alloc)
	: mClient(se)
  , mState(CACHED)
	, mReplyHandler(nullptr)
  , mStartHandler(nullptr)
{
  // Create the single runnable
  std::unique_ptr<T>    up(alloc == nullptr ? new T : alloc());
  mCache = std::move(up);
  if (!mCache) throw std::exception("Can't allocate serial runnable");

  mClient.setResultHandler([this](std::unique_ptr<Poco::Runnable>& r) { receive(r); });
}

template <class T>
bool SerialRunnable<T>::start(const HandlerFunc& f, const bool waitForResult)
{
  // Waiting for a result with an operation currently running is an error.
  if (waitForResult) {
    if (!mCache) return false;

    if (f) f(*(mCache.get()));
    mCache->run();
    std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(mCache));
    mState = RUNNING; // make sure the result handler is triggered
    receive(payload);
    return true;
  }

  mStartHandler = f;

  // If not waiting, then either start or indicate we need to start.
  if (mCache) {
    mState = CACHED;
    return send();
  } else {
    mState = WAITING;
  }
  return true;
}

template <class T>
bool SerialRunnable<T>::send()
{
  if (!mCache) return false;
  if (mStartHandler) {
    mStartHandler(*(mCache.get()));
    mStartHandler = nullptr;
  }
  std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(mCache));
  if (!payload) return false;

  if (!mClient.run(payload)) return false;

  mState = RUNNING;
  return true;
}

template <class T>
void SerialRunnable<T>::receive(std::unique_ptr<Poco::Runnable>& r)
{
	std::unique_ptr<T>		payload(ds::unique_dynamic_cast<T, Poco::Runnable>(r));
	if (payload) {
    if (mState != WAITING) {
		  if (mReplyHandler) mReplyHandler(*(payload.get()));
    }
    mCache = std::move(payload);
  }

  if (mState == WAITING) {
    send();
  } else {
    mState = CACHED;
  }
}

} // namespace ds

#endif // DS_THREAD_SERIALRUNNABLE_H_
