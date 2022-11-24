#include "stdafx.h"

#include "table_table_item.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

namespace downstream {

TableTableItem::TableTableItem(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "table_table_item.xml") {}


void TableTableItem::setData(ds::model::ContentModelRef theData) {
	mData = theData;

	setSpriteText("id", ds::value_to_string(theData.getId()));


	std::stringstream ss;
	ss << "<span weight='bold'>Name:</span>" << theData.getName() << " ";

	for (auto it : theData.getProperties()) {
		ss << "<span weight='bold'>" << it.first << ":</span>" << it.second.getValue() << " ";
	}

	setSpriteText("title", ss.str());

	runLayout();
}


ds::model::ContentModelRef TableTableItem::getData() {
	return mData;
}


} // namespace downstream
