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

	//	q.mData.printTree(true, "");
	});
}

void DataWrangler::runQuery(const bool synchronous) {
	mDataQuery.start(nullptr, synchronous);
}

} // !namespace downstream

