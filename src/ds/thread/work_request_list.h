#pragma once
#ifndef DS_THREAD_WORKREQUESTLIST_H_
#define DS_THREAD_WORKREQUESTLIST_H_

#include <memory>
#include <vector>

namespace ds {

/**
 * \class WorkRequestList
 * \brief Utility to manager a list of WorkRequests, so clients can easily reuse them.
 */
template <typename T>
class WorkRequestList {
  public:
	WorkRequestList(const void* clientId);

	/// Answer the next free item or create one, if I can
	std::unique_ptr<T> next();
	/// Give ownership of the arg to the list.
	void push(std::unique_ptr<T>&);

  private:
	WorkRequestList();

	const void* mClientId;

	std::vector<std::unique_ptr<T>> mRequest;
};

/**
 * implementation
 */
template <class T>
WorkRequestList<T>::WorkRequestList(const void* clientId)
  : mClientId(clientId) {}

template <class T>
std::unique_ptr<T> WorkRequestList<T>::next() {
	std::unique_ptr<T> ans;
	if (!mRequest.empty()) {
		ans = std::move(mRequest.back());
		mRequest.pop_back();
	}
	if (!ans.get()) ans = std::move(std::unique_ptr<T>(new T(mClientId)));
	return ans;
}

template <class T>
void WorkRequestList<T>::push(std::unique_ptr<T>& obj) {
	if (!obj) return;

	try {
		mRequest.push_back(std::move(obj));
	} catch (std::exception const&) {}
}

} // namespace ds

#endif // DS_THREAD_WORKREQUESTLIST_H_
