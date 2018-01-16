#include "stdafx.h"

#include "table_table_item.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include "query/data_wrangler.h"
#include "model/data_model.h"
#include "events/app_events.h"

#include "app/globals.h"

namespace downstream {

TableTableItem::TableTableItem(Globals& g)
	: ds::ui::SmartLayout(g.mEngine, "table_table_item.xml")
	, mGlobals(g)
{
}


void TableTableItem::setData(ds::model::DataModelRef theData) {
	mData = theData;

	setSpriteText("id", ds::value_to_string(theData.getId()));


	std::stringstream ss;
	ss << "<span weight='bold'>Name:</span>" << theData.getName() << " ";

	for(auto it : theData.getProperties()) {
		ss << "<span weight='bold'>"<< it.first << ":</span>" << it.second.getValue() << " ";
	}

	setSpriteText("title", ss.str());

	runLayout();
}


ds::model::DataModelRef TableTableItem::getData() {
	return mData;
}


} // namespace downstream

