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
 * This class favours CPU efficiency over performance: Only ony op
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
	bool							  start(const HandlerFunc& = nullptr, const bool waitForResult = false);

private:
	bool							  send(std::unique_ptr<T>&);
	void								receive(std::unique_ptr<Poco::Runnable>&);

  RunnableClient      mClient;
	// Note the current runnable, so I know what to do with new starts().
	void*							  mCurrent;
  // If start comes in while I have a current, wait until it finishes, then start the next
  std::unique_ptr<T>  mNext;
	std::vector<std::unique_ptr<T>>
                      mCache;
	HandlerFunc				  mReplyHandler;
  std::function<T*(void)>
                      mAlloc;
};

template <class T>
SerialRunnable<T>::SerialRunnable(ui::SpriteEngine& se, const std::function<T*(void)>& alloc)
	: mClient(se)
	, mCurrent(nullptr)
	, mReplyHandler(nullptr)
  , mAlloc(alloc == nullptr ? ([]()->T*{return new T;}) : alloc)
{
	mCache.reserve(4);
  mClient.setResultHandler([this](std::unique_ptr<Poco::Runnable>& r) { receive(r); });
}

template <class T>
bool SerialRunnable<T>::start(const HandlerFunc& f, const bool waitForResult)
{
  // Start a new one by making sure I have a valid mNext. Then it will either
  // sit there waiting to be run, or start immediately.
  if (!mNext) {
	  // Get a new T to run, trying first from the cache.
	  try {
		  if (mCache.size() > 0) {
			  mNext = std::move(mCache.back());
			  mCache.pop_back();
		  }
		  if (mNext.get() == nullptr) mNext = std::move(std::unique_ptr<T>(new T));
	  } catch (std::exception const&) {
	  }
	  if (!mNext) return false;
  }

	if (f) f(*(mNext.get()));

  // Run synchronously if requested
  if (waitForResult) {
    mNext->run();
    mCurrent = mNext.get();
    std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(mNext));
    receive(payload);
    return true;
  }

  // If I'm waiting for someone, keep waiting. Otherwise, go NUTSO.
  if (mCurrent) return true;
  return send(mNext);
}

template <class T>
bool SerialRunnable<T>::send(std::unique_ptr<T>& r)
{
  mCurrent = nullptr;
  if (!r) return false;

	std::unique_ptr<T>			          up = std::move(r);
	if (!up) return false;

  std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(up));
  if (!payload) return false;

  void* ptr = payload.get();
  if (!mClient.run(payload)) return false;

  mCurrent = ptr;
  return true;
}

template <class T>
void SerialRunnable<T>::receive(std::unique_ptr<Poco::Runnable>& r)
{
	std::unique_ptr<T>		payload(ds::unique_dynamic_cast<T, Poco::Runnable>(r));
	if (payload) {
    if (mCurrent && payload.get() == mCurrent) {
		  if (mReplyHandler) mReplyHandler(*(payload.get()));
    }
	  try {
      mCache.push_back(std::move(payload));
	  } catch (std::exception&) {
	  }
  }

  mCurrent = nullptr;

  if (mNext) send(mNext);
}

} // namespace ds

#endif // DS_THREAD_SERIALRUNNABLE_H_
