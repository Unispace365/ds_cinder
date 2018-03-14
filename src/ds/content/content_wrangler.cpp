#include "stdafx.h"

#include "content_wrangler.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>

#include "content_events.h"


namespace ds {

ContentWrangler::ContentWrangler(ds::ui::SpriteEngine& se)
	: mNodeWatcher(se, "localhost", 7777, false)
	, mEngine(se)
	, mContentQuery(se, [] { return new ContentQuery(); }, true)
	, mEventClient(se)
{

	ds::event::Registry::get().addEventCreator(DsNodeMessageReceivedEvent::NAME(), [this]()->ds::Event* {return new DsNodeMessageReceivedEvent(); });
	ds::event::Registry::get().addEventCreator(ContentUpdatedEvent::NAME(), [this]()->ds::Event* {return new ContentUpdatedEvent(); });
	ds::event::Registry::get().addEventCreator(RequestContentQueryEvent::NAME(), [this]()->ds::Event* {return new RequestContentQueryEvent(); });
	mEventClient.listenToEvents<RequestContentQueryEvent>([this](const RequestContentQueryEvent& e) { runQuery(); });

	mNodeWatcher.setDelayedMessageNodeCallback([this](const ds::NodeWatcher::Message& m) {
		mContentQuery.start(nullptr, false);

		for(auto it : m.mData) {
			DsNodeMessageReceivedEvent dnmre;
			dnmre.mUserStringData = it;
			mEngine.getNotifier().notify(dnmre);
		}
	});

	mContentQuery.setReplyHandler([this](ContentQuery& q) {
		DS_LOG_VERBOSE(3, "ContentWrangler: runQuery() complete");

		mEngine.mContent.setName(q.mData.getName());
		mEngine.mContent.setLabel(q.mData.getLabel());
		mEngine.mContent.setId(q.mData.getId());
		mEngine.mContent.setChildren(q.mData.getChildren());

		mEngine.getNotifier().notify(ContentUpdatedEvent());
	});
}

/// This will be called on every hard or soft app restart
void ContentWrangler::initialize() {
	if(!mEngine.getEngineSettings().getBool("content:use_wrangler")) {
		DS_LOG_VERBOSE(4, "ContentWrangler is disabled.");
		return;
	} 

	mModelModelLocation = mEngine.getEngineSettings().getString("content:model_location");

	if(mEngine.getEngineSettings().getBool("content:node_watch")) {
		mNodeWatcher.startWatching();
		DS_LOG_VERBOSE(1, "ContentWrangler::initialize() modelLocation=" << mModelModelLocation << " and using node watcher");
	} else {
		mNodeWatcher.stopWatching();
		DS_LOG_VERBOSE(1, "ContentWrangler::initialize() modelLocation=" << mModelModelLocation << " and NOT using node watcher");
	}

	runQuery();
}

void ContentWrangler::runQuery() {
	if(!mEngine.getEngineSettings().getBool("content:use_wrangler")) {
		return;
	}

	const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
	if(mModelModelLocation.empty() && cms.getDatabasePath().empty()) {
		DS_LOG_VERBOSE(2, "ContentWrangler: no database path or model location specified. Not a problem if you're not using ContentWrangler or Sqlite");
		return;
	}

	DS_LOG_VERBOSE(3, "ContentWrangler: runQuery() starting");

	mContentQuery.start([this](ds::ContentQuery& dq) {
		const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
		dq.mXmlDataModel = mModelModelLocation;
		dq.mCmsDatabase = cms.getDatabasePath();
		dq.mResourceLocation = cms.getResourcePath();
	}, false);
}

} // !namespace downstream

