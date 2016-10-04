#include "delayed_node_watcher.h"

#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>

namespace ds {

/**
* \class ds::DelayedNodeWatcher
*/
DelayedNodeWatcher::DelayedNodeWatcher(ds::ui::SpriteEngine& eng, const std::string& host, const int port)
	: ds::AutoUpdate(eng)
	, mNodeWatcher(eng)
	, mNeedQuery(false)
	, mDelayWaitTime(1.0f)
{

	mLastQueryTime = Poco::Timestamp().epochMicroseconds();

	mNodeWatcher.add([this](const ds::NodeWatcher::Message& msg){
		mNeedQuery = true;
		
		if(mRegularNodeCallback){
			mRegularNodeCallback(msg);
		} else {
			mDelayedMessages.push_back(msg);
		}

		mLastQueryTime = Poco::Timestamp().epochMicroseconds();
	});
}

void DelayedNodeWatcher::addRegularNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback){
	mRegularNodeCallback = callback;
}

void DelayedNodeWatcher::setDelayedNodeCallback(const std::function<void()>& callback){
	mDelayedNodeCallback = callback;
}

void DelayedNodeWatcher::setDelayedMessageNodeCallback(const std::function<void(const NodeWatcher::Message&)>& callback){
	mDelayedMessageNodeCallback = callback;
}

void DelayedNodeWatcher::update(const ds::UpdateParams & p){
	Poco::Timestamp::TimeVal nowwy = Poco::Timestamp().epochMicroseconds();
	float delty = (float)(nowwy - mLastQueryTime) / 1000000.0f;
	if(mNeedQuery && delty > mDelayWaitTime){
		mNeedQuery = false;
		mLastQueryTime = nowwy;
		
		if(mDelayedMessageNodeCallback){
			// send all the stored messages
			for(auto it = mDelayedMessages.begin(); it != mDelayedMessages.end(); it++){
				mDelayedMessageNodeCallback(*it);
			}
		}
		mDelayedMessages.clear();

		if(mDelayedNodeCallback){
			mDelayedNodeCallback();
		}
	}
}

} // !namespace ds
