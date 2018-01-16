#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_TABLE_ITEM
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_TABLE_ITEM


#include <ds/ui/layout/smart_layout.h>
#include "model/data_model.h"

namespace downstream {

class Globals;

/**
* \class downstream::TableTableItem
*			A thing in a table
*/
class TableTableItem : public ds::ui::SmartLayout {
public:
	TableTableItem(Globals& g);


	void								setData(ds::model::DataModelRef theData);
	ds::model::DataModelRef				getData();

private:
	ds::model::DataModelRef				mData;
	Globals&							mGlobals;

};

} // namespace downstream

#endif
#pragma once
