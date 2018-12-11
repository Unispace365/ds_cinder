#pragma once
#ifndef DS_THREAD_ASYNCQUEUE_H_
#define DS_THREAD_ASYNCQUEUE_H_

#include <functional>
#include <vector>
#include <Poco/Mutex.h>

namespace ds {

/**
 * \class AsyncQueue
 * \brief Thread-safe utility for clients to push entries on a queue, and other clients to pull them.
 * I do no memory management, so don't use pointers.
 */
template <typename T>
class AsyncQueue {
public:
	AsyncQueue();

	/// Push can be called from any thread
	void					push(const T&);
	/// Update must be called from a single main update thread (generally the UI thread).
	/// If there have been any messages added, it replies with them. Alternatively, if
	/// an update handler is provided, it runs each message on it.
	const std::vector<T>*	update(const std::function<void(const T&)>& = nullptr);

private:
	AsyncQueue(const AsyncQueue&);

	Poco::Mutex				mMutex;
	std::vector<T>			mLockedQueue;
	std::vector<T>			mUpdateQueue;
};

template <typename T>
AsyncQueue<T>::AsyncQueue()
{
	mLockedQueue.reserve(16);
	mUpdateQueue.reserve(16);
}

template <typename T>
void AsyncQueue<T>::push(const T& t)
{
	try {
		Poco::Mutex::ScopedLock   l(mMutex);
		mLockedQueue.push_back(t);
	} catch (std::exception const&) {
	}
}

template <typename T>
const std::vector<T>* AsyncQueue<T>::update(const std::function<void(const T&)>& fn)
{
	try {
		mUpdateQueue.clear();
		{
			Poco::Mutex::ScopedLock   l(mMutex);
			mUpdateQueue = mLockedQueue;
			mLockedQueue.clear();
		}
		if (mUpdateQueue.empty()) return nullptr;

		if (fn != nullptr) {
			for (auto it=mUpdateQueue.begin(), end=mUpdateQueue.end(); it != end; ++it) {
				fn(*it);
			}
		} else {
			return &mUpdateQueue;
		}

	} catch (std::exception const&) {
	}
	return nullptr;
}

} // namespace ds

#endif // DS_THREAD_ASYNCQUEUE_H_
