#include "stdafx.h"

#include <algorithm>

#include "ds/app/auto_update.h"
#include "ds/app/auto_update_list.h"
#include "ds/params/update_params.h"

namespace ds {

/**
 * \class AutoUpdateList
 */
AutoUpdateList::AutoUpdateList() {
	mRunning.reserve(32);
	mWaiting.reserve(8);
}

void AutoUpdateList::update(const UpdateParams& p) {

	if (!mRunning.empty()) {
		for (auto it : mRunning) {
			it->update(p);
		}
	}

	if (!mWaiting.empty()) {
		for (auto& it : mWaiting) {
			mRunning.push_back(it);
		}
		mWaiting.clear();
	}
}

void AutoUpdateList::addWaiting(AutoUpdate* v) {
	if (!v) return;
	mWaiting.push_back(v);
}

void AutoUpdateList::remove(AutoUpdate* v) {
	if (!v) return;
	mRunning.erase(std::remove(mRunning.begin(), mRunning.end(), v), mRunning.end());
	mWaiting.erase(std::remove(mWaiting.begin(), mWaiting.end(), v), mWaiting.end());
}

} // namespace ds
