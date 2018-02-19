#pragma once
#ifndef DS_APP_ENGINE_ENGINETOUCHQUEUE_H_
#define DS_APP_ENGINE_ENGINETOUCHQUEUE_H_

#include <functional>
#include <vector>
#include <cinder/Thread.h>

#include <ds/debug/logger.h>

namespace ds {

/**
 * \class ds::EngineTouchQueue
 * \brief A very specific class used to help the engine with touch processing. Touch
 * events arrive in a different thread, so this class is provided an external mutex
 * to control locking. Then, in the update, all those events get popped during a
 * single lock, then processed outside of that.
 */
template <typename T>
class EngineTouchQueue {
public:
	EngineTouchQueue(	std::mutex&, float& lastTouchTime,
						const std::function<void(const T&)>&, const std::string& debugLabel = "");

	void					setUpdateFn(const std::function<void(const T&)>&);

	void					setAutoIdleReset(bool);

	// Call this as new events arrive. I will handle locking
	void					incoming(const T&);

	// When updating, first call this while the mutex is locked...
	void					lockedUpdate();
	// ... then call this after the lock has been released.
	void					update(const float currTime);

private:
	EngineTouchQueue(const EngineTouchQueue&);

	std::mutex&				mMutex;
	float&					mLastTouchTime;
	std::function<void(const T&)>
							mUpdateFn;
	std::string				mDebugLabel;

	bool					mAutoIdleReset;

	// Incoming stores the events as they arrive, the
	// Updating holds them temporarily for processing.
	std::vector<T>			mIncoming,
							mUpdating;
};

template <typename T>
EngineTouchQueue<T>::EngineTouchQueue(	std::mutex& m, float& lastTouchTime,
										const std::function<void(const T&)>& updateFn, const std::string& debugLabel)
		: mMutex(m)
		, mLastTouchTime(lastTouchTime)
		, mUpdateFn(updateFn)
		, mDebugLabel(debugLabel)
		, mAutoIdleReset(true)
{
	mIncoming.reserve(32);
	mUpdating.reserve(32);
}

template <typename T>
void EngineTouchQueue<T>::setUpdateFn(const std::function<void(const T&)>& fn) {
	mUpdateFn = fn;
}

template <typename T>
void EngineTouchQueue<T>::setAutoIdleReset(bool autoIdleReset) {
	mAutoIdleReset = autoIdleReset;
}

template <typename T>
void EngineTouchQueue<T>::incoming(const T& t) {
	std::lock_guard<std::mutex> lock(mMutex);
	mIncoming.push_back(t);
}

template <typename T>
void EngineTouchQueue<T>::lockedUpdate() {
	mUpdating.clear();
	mUpdating.swap(mIncoming);
}

template <typename T>
void EngineTouchQueue<T>::update(const float currTime) {
	if (mUpdating.empty()) return;

	if(mAutoIdleReset){
		//DS_LOG_INFO("EngineTouchQueue: losing idle due to " << mDebugLabel);
		mLastTouchTime = currTime;
	}
	
	for (auto it=mUpdating.begin(), end=mUpdating.end(); it!=end; ++it) {
		mUpdateFn(*it);
	}
}

} // namespace ds

#endif // DS_APP_ENGINE_ENGINETOUCHQUEUE_H_
