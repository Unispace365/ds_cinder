#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_NAV_ITEM
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_NAV_ITEM


#include <ds/ui/layout/smart_layout.h>
#include <ds/content/content_model.h>

namespace downstream {

/**
* \class downstream::TableNavItem
*			A thing in a table
*/
class TableNavItem : public ds::ui::SmartLayout {
public:
	TableNavItem(ds::ui::SpriteEngine& eng);

	bool								getExpanded();
	void								setExpanded(const bool isExpanded);

	void								setData(ds::model::ContentModelRef theData);
	ds::model::ContentModelRef			getData();

private:
	bool								mExpanded;
	ds::model::ContentModelRef			mData;

};

} // namespace downstream

#endif
