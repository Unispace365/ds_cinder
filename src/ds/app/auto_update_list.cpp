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
	{
		std::scoped_lock lock(mRunning.mMutex);
		mRunning.mQueue.reserve(128);
	}
	{
		std::scoped_lock lock(mWaiting.mMutex);
		mWaiting.mQueue.reserve(32);
	}
}

void AutoUpdateList::update(const ds::UpdateParams& p) {
	{
		std::scoped_lock lock(mRunning.mMutex);
		for (int i = 0; i < mRunning.mQueue.size(); i++) {
			{
				std::scoped_lock lock2(mRunning.mQueue[i]->mItemMutex);
				auto item = mRunning.mQueue[i];
				if (!item) continue;
				try {
					item->update(p);
				}
				catch (std::exception e) {
					DS_LOG_ERROR("Error in ds_cinder/src/ds/app/auto_update_list.cpp AutoUpdateList::update(..): " << e.what());
				}
			}
		}
	}
	{
		std::scoped_lock lockx(mWaiting.mMutex);
		if (!mWaiting.mQueue.empty()) {
			//for (auto it = mWaiting.mQueue.begin(), end = mWaiting.mQueue.end(); it != end; ++it) {
			for (int i = 0; i < mWaiting.mQueue.size(); i++) {
				{
					std::scoped_lock lockx2(mRunning.mMutex);
					std::scoped_lock lockx3(mWaiting.mQueue[i]->mItemMutex);
					mRunning.mQueue.push_back(mWaiting.mQueue[i]);
				}
			}
		}
		mWaiting.mQueue.clear();
	}
}

void AutoUpdateList::addWaiting(AutoUpdate* v) {
	if (!v) return;
	{
		std::scoped_lock lock(mWaiting.mMutex);
		//std::scoped_lock lock2(v->mBenMutex);
		mWaiting.mQueue.push_back(v);
	}
}

void AutoUpdateList::remove(AutoUpdate* v) {
	if (!v) return;
	{
		std::scoped_lock lock(v->mItemMutex);
		{
			//std::scoped_lock lockee(mRunning.mMutex);
			mRunning.mQueue.erase(std::remove(mRunning.mQueue.begin(), mRunning.mQueue.end(), v), mRunning.mQueue.end());
		}
		{
			//std::scoped_lock locky(mWaiting.mMutex);
			mWaiting.mQueue.erase(std::remove(mWaiting.mQueue.begin(), mWaiting.mQueue.end(), v), mWaiting.mQueue.end());
		}
	}
}

} // namespace ds
