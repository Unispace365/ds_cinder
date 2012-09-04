#pragma once
#ifndef DS_THREAD_PARALLELRUNNABLE_H_
#define DS_THREAD_PARALLELRUNNABLE_H_

#include <assert.h>
#include <functional>
#include <vector>
#include "ds/thread/runnable_client.h"
#include "ds/util/memory_ds.h"

namespace ds {

/* DS::PARALLEL-RUNNABLE
 * A utility for running any number of async operations at a time.  Keep
 * starting operations and you'll keep getting their results. Clients
 * define the type through the template, set a function to handle any
 * replies, then kick of a query with start().
 *
 *** IMPLICIT INTERFACE
 *
 * T is a subclass of Poco::Runnable
 ******************************************************************/
template <class T>
class ParallelRunnable {
public:
	typedef std::function<void (T&)>	HandlerFunc;

public:
	ParallelRunnable(ui::SpriteEngine&);
	
	// Start a new runnable, intializing it via the handler block.
	bool							    start(const HandlerFunc& = nullptr);
	void							    setReplyHandler(const HandlerFunc& f) { mReplyHandler = f; }

private:
	void								  receive(std::unique_ptr<Poco::Runnable>&);

  RunnableClient			  mClient;
	std::vector<std::unique_ptr<T>>
                        mCache;
	HandlerFunc						mReplyHandler;
};

template <class T>
ParallelRunnable<T>::ParallelRunnable(ui::SpriteEngine& se)
	: mClient(se)
	, mReplyHandler(nullptr)
{
	mCache.reserve(4);
  mClient.setResultHandler([this](std::unique_ptr<Poco::Runnable>& r) { receive(r); });
}

template <class T>
bool ParallelRunnable<T>::start(const HandlerFunc& f)
{
	// Get a new T to run.  Try to take from the cache.
	unique_ptr<T>			up;
	try {
		if (mCache.size() > 0) {
			up = std::move(mCache.back());
			mCache.pop_back();
		}
		if (up.get() == nullptr) up = std::move(unique_ptr<T>(new T));
	} catch (std::exception const&) {
	}
	if (up.get() == nullptr) return false;

	if (f) f(*(up.get()));

  std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(up));
  if (!payload) return false;
  return mClient.run(payload);
}

template <class T>
void ParallelRunnable<T>::receive(std::unique_ptr<Poco::Runnable>& r)
{
	std::unique_ptr<T>		payload(ds::unique_dynamic_cast<T, Poco::Runnable>(r));
	if (!payload) {
		assert(false);
	} else {
  	if (mReplyHandler) mReplyHandler(*(payload.get()));
    try {
      mCache.push_back(std::move(payload));
    } catch (std::exception const&) {
    }
	}
}

} // namespace ds

#endif // DS_THREAD_PARALLELRUNNABLE_H_
