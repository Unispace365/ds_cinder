#include "query_handler.h"

#include "app/globals.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cinder/Json.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>

namespace example {

/**
 * \class example::QueryHandler
 */
QueryHandler::QueryHandler(ds::ui::SpriteEngine& se, AllData& ad)
  : mEventClient(se.getNotifier(),
				 [this](const ds::Event* e) {
					 if (e) onAppEvent(*e);
				 })
  , mAllData(ad)
  , mNodeWatcher(se)
  , mStoryQuery(se, []() { return new StoryQuery(); }) {

	// Initialize data
	mStoryQuery.setReplyHandler([this](StoryQuery& q) { this->onStoryQuery(q); });

	mNodeWatcher.setDelayedMessageNodeCallback([this](const ds::NodeWatcher::Message& m) { runInitialQueries(false); });
}

void QueryHandler::runInitialQueries(const bool synchronous) {
	mStoryQuery.start(nullptr, synchronous);
}

void QueryHandler::onAppEvent(const ds::Event& _e) {
	// Optionally handle app events to re - query if needed
}

void QueryHandler::onStoryQuery(StoryQuery& q) {
	mAllData.mStories = q.mOutput.mStories;

	// In general, when a view is re-loaded, it'll pick up the new stories after this point.
	// You can also dispatch an event here to notify views of new data
	// mEventClient.notify(StoryDataLoadedEvent());
}


} // namespace example
