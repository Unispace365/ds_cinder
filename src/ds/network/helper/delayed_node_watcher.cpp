#include "stdafx.h"

#include "delayed_node_watcher.h"


namespace ds {

/**
* \class ds::DelayedNodeWatcher
*/
DelayedNodeWatcher::DelayedNodeWatcher(ds::ui::SpriteEngine& eng, const std::string& host, const int port, const bool autostart)
	: ds::AutoUpdate(eng)
	, mNodeWatcher(eng, host, port, autostart)
	, mNeedQuery(false)
	, mDelayWaitTime(1.0f)
{

	mLastQueryTime = Poco::Timestamp().epochMicroseconds();

	mNodeWatcher.add([this](const ds::NodeWatcher::Message& msg) {
		mNeedQuery = true;

		if(mRegularNodeCallback) {
			mRegularNodeCallback(msg);
		} else {
			for(auto it : msg.mData) {
				mDelayedMessages.mData.push_back(it);
			}
		}

		mLastQueryTime = Poco::Timestamp().epochMicroseconds();
	});
}

void DelayedNodeWatcher::addRegularNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback) {
	mRegularNodeCallback = callback;
}

void DelayedNodeWatcher::setDelayedNodeCallback(const std::function<void()>& callback) {
	mDelayedNodeCallback = callback;
}

void DelayedNodeWatcher::setDelayedMessageNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback) {
	mDelayedMessageNodeCallback = callback;
}

void DelayedNodeWatcher::startWatching() {
	mNodeWatcher.startWatching();
}

void DelayedNodeWatcher::stopWatching() {
	mNodeWatcher.stopWatching();
}

void DelayedNodeWatcher::update(const ds::UpdateParams & p) {
	Poco::Timestamp::TimeVal nowwy = Poco::Timestamp().epochMicroseconds();
	float delty = (float)(nowwy - mLastQueryTime) / 1000000.0f;
	if(mNeedQuery && delty > mDelayWaitTime) {
		mNeedQuery = false;
		mLastQueryTime = nowwy;

		if(mDelayedMessageNodeCallback) {
			mDelayedMessageNodeCallback(mDelayedMessages);

		}
		mDelayedMessages = NodeWatcher::Message();

		if(mDelayedNodeCallback) {
			mDelayedNodeCallback();
		}
	}
}

} // !namespace ds
