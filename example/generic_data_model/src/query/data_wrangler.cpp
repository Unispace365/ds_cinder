#include "stdafx.h"

#include "data_wrangler.h"

#include <ds/app/event_notifier.h>

#include "events/app_events.h"

namespace downstream {

/**
* \class downstream::DataWrangler
*/
DataWrangler::DataWrangler(ds::ui::SpriteEngine& se)
	: mNodeWatcher(se)
	, mEngine(se)
	, mDataQuery(se, [] { return new DataQuery(); })
{

	mNodeWatcher.setDelayedMessageNodeCallback([this](const ds::NodeWatcher::Message& m) {
		mDataQuery.start(nullptr, false);
	});

	mDataQuery.setReplyHandler([this](DataQuery& q) {
		mData = q.mData;

		mEngine.getNotifier().notify(DataUpdatedEvent());
	});
}

void DataWrangler::runQuery() {
	mDataQuery.start([this](downstream::DataQuery& dq) {
		const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
		dq.mXmlDataModel = "%APP%/data/model/data_model.xml"; 
		dq.mCmsDatabase = cms.getDatabasePath();
		dq.mResourceLocation = cms.getResourcePath();
	}, false);
}

} // !namespace downstream

