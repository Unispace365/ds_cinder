#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_VIEW
#define _GENERIC_DATA_MODEL_APP_UI_TABLE_TABLE_VIEW


#include <ds/content/content_model.h>
#include <ds/ui/layout/smart_layout.h>

namespace downstream {
class TableNavItem;
class TableTableItem;

/**
 * \class downstream::TableView
 *			A table of things from a data model
 */
class TableView : public ds::ui::SmartLayout {
  public:
	TableView(ds::ui::SpriteEngine& eng);

  private:
	void setData();
	void addNavItem(ds::ui::Sprite* parenty, const float indent, ds::model::ContentModelRef theModel);

	void setTableData(ds::model::ContentModelRef theModel);

	std::vector<TableNavItem*> mNavItems;

	std::vector<TableTableItem*> mTableItems;
};

} // namespace downstream

#endif
