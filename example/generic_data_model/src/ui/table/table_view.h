#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_VIEW
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_VIEW


#include <ds/ui/layout/smart_layout.h>
#include "model/data_model.h"

namespace downstream {
class TableNavItem;
class TableTableItem;
class Globals;

/**
* \class downstream::TableView
*			A table of things from a data model
*/
class TableView : public ds::ui::SmartLayout {
public:
	TableView(Globals& g);

private:
	void								setData();
	void								addNavItem(ds::ui::Sprite* parenty, const float indent, ds::model::DataModelRef theModel, const std::string& childrenName);

	void								setTableData(ds::model::DataModelRef theModel, const std::string& childrenName);

	std::vector<TableNavItem*>			mNavItems;

	std::vector<TableTableItem*>		mTableItems;

	Globals&							mGlobals;

};

} // namespace downstream

#endif
