#pragma once
#ifndef DS_THREAD_TIMEDRUNNABLE_H_
#define DS_THREAD_TIMEDRUNNABLE_H_

#include <assert.h>
#include <functional>
#include <vector>
#include <Poco/Timestamp.h>
#include "ds/thread/runnable_client.h"
#include "ds/util/memory_ds.h"

namespace ds {

/* DS::TIMED-RUNNABLE
 * A utility for clients that have a runnable object they want to
 * schedule for regular intervals.  Supply an interval between runs
 * and a Runnable T to perform the run, and then call update() in
 * the main loop to cause the run to schedule, when necessary.
 *
 *** IMPLICIT INTERFACE
 *
 * T is a subclass of Poco::Runnable (where run() runs in a worker
 * thread).
 *
 * T implements T::finished() (which is called from the thread
 * calling update()).
 ******************************************************************/
template <class T>
class TimedRunnable
{
public:
	// Clients need to tell the time (in seconds) between runs.
	// Clients also need to supply the instance of the class that gets run.
	TimedRunnable(ui::SpriteEngine&, const double interval, T * payload);

	void								      update();
	void								      update(const Poco::Timestamp::TimeVal&);
  // This will cause an update as soon as the payload is available
  void                      requestUpdate();

	// Set the interval (in seconds) between updates.  This won't affect the current
	// update, which will happen according to the pre-changed interval.
	void								      setInterval(const double interval);
	Poco::Timestamp::TimeVal  getIntervalMu() const;

  // Set a callback when I'm about to start processing (called from update() thread).
  void                      setOnStartFn(const std::function<void(T&)>&);

	// Answer 0 - 1, where 0 is the start of an update, and 1 is the next update.
	double								    getProgress() const;
	double								    getProgress(const Poco::Timestamp::TimeVal&) const;
    
private:
	void								      receive(std::unique_ptr<Poco::Runnable>&);

	RunnableClient						mClient;
	Poco::Timestamp::TimeVal  mInterval, mNextUpdate;
	std::unique_ptr<T>				mPayload;
  std::function<void(T&)>   mOnStartFn;
};

/* DS::TIMED-RUNNABLE impl
 ******************************************************************/
template <class T>
TimedRunnable<T>::TimedRunnable(ui::SpriteEngine& se, const double interval, T * payload)
	: mClient(se)
	, mNextUpdate(0)
  , mOnStartFn(nullptr)
{
	setInterval(interval);
  mClient.setResultHandler([this](std::unique_ptr<Poco::Runnable>& r) { receive(r); });
	// The data I will send and receive
	mPayload = std::move(std::unique_ptr<T>(payload));
}

template <class T>
void TimedRunnable<T>::update()
{
	update(Poco::Timestamp().epochMicroseconds());
}

template <class T>
void TimedRunnable<T>::update(const Poco::Timestamp::TimeVal& v)
{
	if (v >= mNextUpdate && mPayload.get()) {
    if (mOnStartFn) {
      T*                              p = mPayload.get();
      if (p) mOnStartFn(*p);
    }
	  std::unique_ptr<Poco::Runnable>		payload(ds::unique_dynamic_cast<Poco::Runnable, T>(mPayload));
    if (!payload) return;
    mClient.run(payload);
		mNextUpdate = v + mInterval;
	}
}

template <class T>
void TimedRunnable<T>::requestUpdate()
{
  mNextUpdate = Poco::Timestamp().epochMicroseconds();
}

template <class T>
void TimedRunnable<T>::setInterval(const double interval)
{
	mInterval = static_cast<Poco::Timestamp::TimeVal>(interval*1000000.0);
}

template <class T>
Poco::Timestamp::TimeVal TimedRunnable<T>::getIntervalMu() const
{
	return mInterval;
}

template <class T>
void TimedRunnable<T>::setOnStartFn(const std::function<void(T&)>& fn)
{
  mOnStartFn = fn;
}

template <class T>
double TimedRunnable<T>::getProgress() const
{
	return getProgress(Poco::Timestamp().epochMicroseconds());
}

template <class T>
double TimedRunnable<T>::getProgress(const Poco::Timestamp::TimeVal& v) const
{
	if (mInterval <= 0) return 0;
	const double						pos = static_cast<double>(v - (mNextUpdate - mInterval));
	return pos / static_cast<double>(mInterval);
}

template <class T>
void TimedRunnable<T>::receive(std::unique_ptr<Poco::Runnable>& r)
{
	std::unique_ptr<T>		payload(ds::unique_dynamic_cast<T, Poco::Runnable>(r));
	if (!payload) {
		assert(false);
	} else {
		payload->finished();
		mPayload = std::move(payload);
	}
}

} // namespace ds

#endif // DS_THREAD_TIMEDRUNNABLE_H_