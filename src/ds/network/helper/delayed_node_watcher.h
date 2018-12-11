#ifndef DS_NETWORK_HELPER_DELAYED_NODE_WATCHER
#define DS_NETWORK_HELPER_DELAYED_NODE_WATCHER

#include <Poco/Timestamp.h>

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/network/node_watcher.h>
#include <ds/app/auto_update.h>

namespace ds {

/**
* \class DelayedNodeWatcher
* \brief Wraps NodeWatcher to implement a delay on messages, so if many messages come down the pipe in quick succession, the query only happens after the end of the burst
*			This prevents querying a lot of times unnecessarily.
*/
class DelayedNodeWatcher : public ds::AutoUpdate {
public:
	DelayedNodeWatcher(ds::ui::SpriteEngine&, const std::string& host = "localhost", const int port = 7777, const bool autostart = true);

	/** Calls back directly from NodeWatcher, without any delay */
	void								addRegularNodeCallback(const std::function<void(const NodeWatcher::Message&)>&);

	/** Waits until there is a gap in messages to callback. No message-specific info here, generally just query everything from this message */
	void								setDelayedNodeCallback(const std::function<void()>&);

	/** Waits until there is a gap in messages to callback. This one has message info in case you want it to query specific tables */
	void								setDelayedMessageNodeCallback(const std::function<void(const NodeWatcher::Message&)>&);

	/** How long to wait since the last message to send out a callback. Delay is extended if a node message comes in before the delay is up. */
	void								setDelayTime(const float newDelay) { mDelayWaitTime = newDelay; };

	/** Starts listening on UDP host and port. Does nothing if already watching */
	void								startWatching();

	/** Stops listening. Does nothing if already not listening */
	void								stopWatching();

protected:
	virtual void						update(const ds::UpdateParams &);

private:

	ds::NodeWatcher						mNodeWatcher;

	bool								mNeedQuery;
	Poco::Timestamp::TimeVal			mLastQueryTime;

	NodeWatcher::Message				mDelayedMessages;

	std::function<void(const NodeWatcher::Message&)>	mRegularNodeCallback;
	std::function<void()>								mDelayedNodeCallback;
	std::function<void(const NodeWatcher::Message&)>	mDelayedMessageNodeCallback;

	float								mDelayWaitTime;
};

} // !namespace ds

#endif // !DS_NETWORK_HELPER_DELAYED_NODE_WATCHER#pragma once
