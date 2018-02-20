#include "stdafx.h"

#include "query_handler.h"

#include <ds/app/event_notifier.h>
#include "app/globals.h"
#include "events/app_events.h"

namespace fullstarter {

QueryHandler::QueryHandler(ds::ui::SpriteEngine& se, AllData &ad)
		: mEventClient(se.getNotifier(), [this](const ds::Event* e){if (e) onAppEvent(*e); })
		, mAllData(ad)
		, mNodeWatcher(se)
		, mStoryQuery(se, [](){return new StoryQuery(); })
{

	// Initialize data
	mStoryQuery.setReplyHandler([this](StoryQuery& q){
		mAllData.mStories = q.mOutput.mStories;
		mEventClient.notify(StoryDataUpdatedEvent());
	});

	mNodeWatcher.setDelayedMessageNodeCallback([this](const ds::NodeWatcher::Message& m){
		runQueries();
	});
}

void QueryHandler::runQueries(){
	mStoryQuery.start(nullptr, false);
}

void QueryHandler::onAppEvent(const ds::Event& in_e) {
	// Optionally handle app events to re - query if needed
}

} // !namespace fullstarter
