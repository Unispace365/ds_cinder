#include "stdafx.h"

#include "ds/app/auto_update_list.h"

#include "ds/app/auto_update.h"
#include "ds/params/update_params.h"
#include <Poco/Timestamp.h>
#include <algorithm>

namespace ds {

/**
 * \class AutoUpdateList
 */
AutoUpdateList::AutoUpdateList() {
	mRunning.reserve(32);
	mWaiting.reserve(8);
}

void AutoUpdateList::update(const ds::UpdateParams& p) {
	if (!mWaiting.empty()) {
		for (auto it = mWaiting.begin(), end = mWaiting.end(); it != end; ++it) {
			mRunning.push_back(*it);
		}
		mWaiting.clear();
	}
	if (mRunning.empty()) return;

	for (auto it : mRunning) {
		it->update(p);
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
