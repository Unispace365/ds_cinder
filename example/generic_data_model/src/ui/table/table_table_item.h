#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_TABLE_ITEM
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_TABLE_ITEM


#include <ds/ui/layout/smart_layout.h>
#include <ds/content/content_model.h>

namespace downstream {

/**
* \class downstream::TableTableItem
*			A thing in a table
*/
class TableTableItem : public ds::ui::SmartLayout {
public:
	TableTableItem(ds::ui::SpriteEngine& eng);


	void								setData(ds::model::ContentModelRef theData);
	ds::model::ContentModelRef			getData();

private:
	ds::model::ContentModelRef			mData;

};

} // namespace downstream

#endif
#pragma once
