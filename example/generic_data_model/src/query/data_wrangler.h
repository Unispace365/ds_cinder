#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_QUERY_DATA_WRANGLER
#define _GENERIC_DATA_MODEL_APP_QUERY_DATA_WRANGLER

#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/network/helper/delayed_node_watcher.h>

#include "model/data_model.h"
#include "query/data_query.h"

namespace downstream {

/**
* \class downstream::DataWrangler
* \brief Handle app events that deal with querying for data.
*/
class DataWrangler {
public:
	DataWrangler(ds::ui::SpriteEngine&);

	ds::model::DataModelRef				mData;

	void								runQuery(const bool synchronous);

private:
	ds::ui::SpriteEngine&				mEngine;
	ds::SerialRunnable<DataQuery>		mDataQuery;

	ds::DelayedNodeWatcher				mNodeWatcher;
};

} // !namespace downstream

#endif 
