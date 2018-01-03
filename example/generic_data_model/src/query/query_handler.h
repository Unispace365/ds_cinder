#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_QUERY_QUERYHANDLER_H_
#define _GENERIC_DATA_MODEL_APP_QUERY_QUERYHANDLER_H_

#include <ds/app/event_client.h>
#include <ds/data/resource_list.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>
// NOTE: Placing this include at the top gets a conflict
// with cinder. Need to look into that.
#include <ds/network/helper/delayed_node_watcher.h>
#include "query/story_query.h"
#include "model/all_data.h"
#include "query/data_query.h"

namespace downstream {

/**
 * \class downstream::QueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class QueryHandler {
public:
	QueryHandler(ds::ui::SpriteEngine&, AllData&);

	void								runInitialQueries(const bool synchronous);

private:

	void								onAppEvent(const ds::Event&);
	void								onStoryQuery(StoryQuery&);

	ds::EventClient						mEventClient;

	AllData&							mAllData;

	ds::SerialRunnable<StoryQuery>		mStoryQuery;
	ds::SerialRunnable<DataQuery>		mDataQuery;

	ds::DelayedNodeWatcher				mNodeWatcher;
};

} // !namespace downstream

#endif // !_GENERIC_DATA_MODEL_APP_QUERY_QUERYHANDLER_H_
