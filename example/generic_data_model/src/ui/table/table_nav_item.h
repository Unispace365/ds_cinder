#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_NAV_ITEM
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_NAV_ITEM


#include <ds/ui/layout/smart_layout.h>
#include "model/data_model.h"

namespace downstream {

class Globals;

/**
* \class downstream::TableNavItem
*			A thing in a table
*/
class TableNavItem : public ds::ui::SmartLayout {
public:
	TableNavItem(Globals& g);

	bool								getExpanded();
	void								setExpanded(const bool isExpanded);

	void								setData(ds::model::DataModelRef theData);
	ds::model::DataModelRef				getData();

private:
	bool								mExpanded;
	ds::model::DataModelRef				mData;
	Globals&							mGlobals;

};

} // namespace downstream

#endif
