#include "stdafx.h"

#include "table_nav_item.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

namespace downstream {

TableNavItem::TableNavItem(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "table_nav_item.xml")
  , mExpanded(false) {}

bool TableNavItem::getExpanded() {
	return mExpanded;
}

void TableNavItem::setExpanded(const bool isExpanded) {
	mExpanded = isExpanded;
	if (mData.hasChildren()) {
		if (mExpanded) {
			setSpriteText("id", "-");
		} else {
			setSpriteText("id", "+");
		}
	}
	runLayout();
}

void TableNavItem::setData(ds::model::ContentModelRef theData) {
	mData = theData;

	if (mData.hasChildren()) {
		setSpriteText("id", "+");
	} else {
		setSpriteText("id", " ");
	}
	setSpriteText("title", "<span weight='bold'>" + theData.getName() + "</span> | <span weight='light'>" +
							   theData.getLabel() + "</span>");

	runLayout();
}


ds::model::ContentModelRef TableNavItem::getData() {
	return mData;
}


} // namespace downstream
