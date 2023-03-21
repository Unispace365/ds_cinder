#pragma once
#ifndef _PANGO_APP_QUERY_QUERYHANDLER_H_
#define _PANGO_APP_QUERY_QUERYHANDLER_H_

#include <ds/app/event_client.h>
#include <ds/data/resource_list.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>
// NOTE: Placing this include at the top gets a conflict
// with cinder. Need to look into that.
#include "model/all_data.h"
#include "query/story_query.h"

namespace pango {

/**
 * \class pango::QueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class QueryHandler {
  public:
	QueryHandler(ds::ui::SpriteEngine&, AllData&);

	void runInitialQueries(const bool synchronous);

  private:
	void onAppEvent(const ds::Event&);
	void onStoryQuery(StoryQuery&);

	ds::EventClient mEventClient;

	AllData& mAllData;

	ds::SerialRunnable<StoryQuery> mStoryQuery;
};

} // namespace pango

#endif // !_PANGO_APP_QUERY_QUERYHANDLER_H_
