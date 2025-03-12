#include "stdafx.h"

#include "content_wrangler.h"

#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "content_events.h"


namespace ds {

ContentWrangler::ContentWrangler(ui::SpriteEngine& se)
  : mEngine(se)
  , mContentQuery(se, [] { return new ContentQuery(); })
  , mNodeWatcher(se, "localhost", 7777, false)
  , mEventClient(se) {

	mEngine.mContent.setName("root");
	mEngine.mContent.setLabel("The root of all content");

	event::Registry::get().addEventCreator(DsNodeMessageReceivedEvent::NAME(), []() -> Event* { return new DsNodeMessageReceivedEvent(); });
	event::Registry::get().addEventCreator(ContentUpdatedEvent::NAME(), []() -> Event* { return new ContentUpdatedEvent(); });
	event::Registry::get().addEventCreator(RequestContentQueryEvent::NAME(), []() -> Event* { return new RequestContentQueryEvent(); });
	mEventClient.listenToEvents<RequestContentQueryEvent>([this](const RequestContentQueryEvent& e) { runQuery(); });

	mNodeWatcher.setDelayedMessageNodeCallback([this](const NodeWatcher::Message& m) {
		// mContentQuery.start(nullptr);
		runQuery();

		for (const auto& it : m.mData) {
			DsNodeMessageReceivedEvent dnmre;
			dnmre.mUserStringData = it;
			mEngine.getNotifier().notify(dnmre);
		}
	});

	mContentQuery.setReplyHandler([this](ContentQuery& q) { receiveQuery(q); });
}

void ContentWrangler::receiveQuery(ContentQuery& q) const {
	if (q.mData.empty()) {
		DS_LOG_WARNING("ContentWrangler: runQuery() completed with no data.");
		return;
	}
	DS_LOG_VERBOSE(3, "ContentWrangler: runQuery() complete");

	if (auto match = mEngine.mContent.getChildByName(q.mData.getName())) {
		using ModelVec	   = std::vector<model::ContentModelRef>;
		ModelVec newTables = q.mData.getChildren();

		if (mEngine.mContent.getChildByName("sqlite").getPropertyBool("merge_content")) {
			// Merge new tables with the existing data tables
			ModelVec existingTables = match.getChildren();
			ModelVec mergedList;
			for (const auto& tit : existingTables) {

				bool foundNewTable = false;
				// look for updates to this table, and if there are, remove them from the new list
				for (auto nit = newTables.begin(); nit < newTables.end(); ++nit) {
					if ((*nit).getName() == tit.getName()) {
						mergedList.emplace_back((*nit));
						foundNewTable = true;
						// remove from the new list so when we add the remainders later, it's there
						newTables.erase(nit);
						break;
					}
				}

				/// One of the existing tables didn't get updated, so add it to the merged list
				if (!foundNewTable) {
					mergedList.emplace_back(tit);
				}
			}

			/// Add any new tables that weren't already in the existing list
			for (const auto& nit : newTables) {
				mergedList.emplace_back(nit);
			}

			/// replace all children of the top-level node
			match.setChildren(mergedList);
		} else {
			// Just straight up replace, no merge
			match.clear();
			match = q.mData;
		}
	} else {
		mEngine.mContent.addChild(q.mData);
	}

	mEngine.getNotifier().notify(ContentUpdatedEvent());
}

/// This will be called on every hard or soft app restart
void ContentWrangler::initialize() {
	if (!mEngine.getEngineSettings().getBool("content:use_wrangler")) {
		DS_LOG_VERBOSE(4, "ContentWrangler is disabled.");
		return;
	}

	mModelModelLocation = mEngine.getEngineSettings().getString("content:model_location", 0, "");

	if (mEngine.getEngineSettings().getBool("content:node_watch")) {
		mNodeWatcher.startWatching();
		DS_LOG_VERBOSE(1, "ContentWrangler::initialize() modelLocation=" << mModelModelLocation << " and using node watcher");
	} else {
		mNodeWatcher.stopWatching();
		DS_LOG_VERBOSE(1, "ContentWrangler::initialize() modelLocation=" << mModelModelLocation << " and NOT using node watcher");
	}

	runQuery();
}

void ContentWrangler::runQuery() {
	if (!mEngine.getEngineSettings().getBool("content:use_wrangler")) {
		return;
	}

	if (mModelModelLocation.empty()) {
		DS_LOG_VERBOSE(2, "ContentWrangler: no model location specified. Not a problem if you're not using "
						  "ContentWrangler or Sqlite");
		return;
	}

	DS_LOG_VERBOSE(3, "ContentWrangler: runQuery() starting");

	auto allModels = split(mModelModelLocation, ";", true);
	for (const auto& it : allModels) {
		mContentQuery.start([it](ContentQuery& dq) {
			const Resource::Id cms(Resource::Id::CMS_TYPE, 0);
			dq.mXmlDataModel	 = it;
			dq.mCmsDatabase		 = cms.getDatabasePath();
			dq.mResourceLocation = cms.getResourcePath();
		});
	}
}


} // namespace ds
