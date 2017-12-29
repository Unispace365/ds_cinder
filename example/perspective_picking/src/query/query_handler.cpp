#include "query_handler.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cinder/Json.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include "app/app_defs.h"
#include "app/globals.h"

namespace perspective_picking {

/**
 * \class perspective_picking::QueryHandler
 */
QueryHandler::QueryHandler(ds::ui::SpriteEngine& se, AllData &ad)
		: mEventClient(se.getNotifier(), [this](const ds::Event* e){if (e) onAppEvent(*e); })
		, mAllStories(ad.mAllStories)
		, mStoryQuery(se, [](){return new StoryQuery(); })
{

	// Initialize data
	mStoryQuery.setReplyHandler([this](StoryQuery& q){this->onStoryQuery(q); });
	mStoryQuery.start(nullptr, true);
}

void QueryHandler::onAppEvent(const ds::Event& _e) {
}

void QueryHandler::onStoryQuery(StoryQuery& q) {
	mAllStories = q.mOutput;
}


} // !namespace perspective_picking


