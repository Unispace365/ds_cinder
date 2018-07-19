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

	mEngine.mContent.setName("root");
	mEngine.mContent.setLabel("The root of all content");

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

		if(mEngine.mContent.hasDirectChild(q.mData.getName())) {
			auto theChildren = mEngine.mContent.getChildren();

			for (auto& it : theChildren){
				
				// Find the top-level node that matches the incoming data
				if(it.getName() == q.mData.getName()) {

					// Merge new tables with the existing data tables
					std::vector<ds::model::ContentModelRef> existingTables = it.getChildren();
					std::vector<ds::model::ContentModelRef> newTables = q.mData.getChildren();
					std::vector<ds::model::ContentModelRef> mergedList;

					for (auto tit : existingTables){

						bool foundNewTable = false;
						// look for updates to this table, and if there are, remove them from the new list
						for(auto nit = newTables.begin(); nit < newTables.end(); nit++) {
							if((*nit).getName() == tit.getName()) {
								mergedList.emplace_back((*nit));
								foundNewTable = true;
								// remove from the new list so when we add the remainders later, it's there
								newTables.erase(nit);
								break;
							}
						}

						/// One of the existing tables didn't get updated, so add it to the merged list
						if(!foundNewTable) {
							mergedList.emplace_back(tit);
						}
					}

					/// Add any new tables that weren't already in the existing list
					for (auto nit : newTables){
						mergedList.emplace_back(nit);
					}

					/// replace all children of the top-level node
					it.setChildren(mergedList);
					break;
				}
			}
		} else {
			mEngine.mContent.addChild(q.mData);
		}

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

	if(mModelModelLocation.empty()) {
		DS_LOG_VERBOSE(2, "ContentWrangler: no model location specified. Not a problem if you're not using ContentWrangler or Sqlite");
		return;
	}

	DS_LOG_VERBOSE(3, "ContentWrangler: runQuery() starting");

	auto allModels = ds::split(mModelModelLocation, ";", true);
	for(auto it : allModels) {
		auto thisModel = it;
		mContentQuery.start([this, thisModel](ds::ContentQuery& dq) {
			const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
			dq.mXmlDataModel = thisModel;
			dq.mCmsDatabase = cms.getDatabasePath();
			dq.mResourceLocation = cms.getResourcePath();
		}, false);
	}
}

} // !namespace downstream

