#pragma once
#ifndef _FULLSTARTER_APP_QUERY_QUERYHANDLER_H_
#define _FULLSTARTER_APP_QUERY_QUERYHANDLER_H_

#include <ds/network/helper/delayed_node_watcher.h>
#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>
#include "query/story_query.h"
#include "model/all_data.h"

namespace fullstarter {

/**
 * \class fullstarter::QueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class QueryHandler {
public:
	QueryHandler(ds::ui::SpriteEngine&, AllData&);

	void								runQueries();

private:

	void								onAppEvent(const ds::Event&);

	ds::EventClient						mEventClient;

	AllData&							mAllData;

	ds::SerialRunnable<StoryQuery>		mStoryQuery;

	ds::DelayedNodeWatcher				mNodeWatcher;
};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_QUERY_QUERYHANDLER_H_