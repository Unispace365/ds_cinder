#include "stdafx.h"

#include "table_view.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"
#include "query/data_wrangler.h"
#include "model/data_model.h"
#include "events/app_events.h"

#include "table_nav_item.h"
#include "table_table_item.h"

namespace downstream {

TableView::TableView(Globals& g)
	: ds::ui::SmartLayout(g.mEngine, "table_view.xml")
	, mGlobals(g)
{

	setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());

	listenToEvents<DataUpdatedEvent>([this](const DataUpdatedEvent& ev) {
		setData();
	});

	setData();


}

void TableView::setData() {
	for (auto it : mNavItems){
		it->release();
	}

	mNavItems.clear();

	auto holder = getSprite("nav_layout");
	if(holder) {
		addNavItem(holder, 0.0f, mGlobals.mDataWrangler.mData, "");
	}

	runLayout();
}


void TableView::addNavItem(ds::ui::Sprite* parenty, const float indent, ds::model::DataModelRef theModel, const std::string& childrenName) {
	if(!parenty) return;
	TableNavItem* tvi = new TableNavItem(mGlobals);
	tvi->mLayoutLPad = indent;

	if(childrenName.empty()) {
		tvi->setData(theModel);
	} else {
		tvi->setData(theModel, childrenName);
		tvi->setTapCallback([this, tvi, childrenName, parenty, theModel](ds::ui::Sprite* bs, const ci::vec3& pos) {

			auto oldItems = mNavItems;
			mNavItems.clear();
			if(tvi->getExpanded()) {
				tvi->setExpanded(false);

				std::vector<TableNavItem*> deletingItems;

				bool deleting = false;
				for(auto it : oldItems) {
					if(it == tvi) {
						deleting = true;
						mNavItems.emplace_back(it);
					} else if(deleting){
						if(it->mLayoutLPad > tvi->mLayoutLPad) {
							deletingItems.emplace_back(it);
						} else {
							mNavItems.emplace_back(it);
							deleting = false;
						}
					} else {
						mNavItems.emplace_back(it);
					}
				}

				for (auto it : deletingItems){
					it->release();
				}


			} else {

				for(auto it : oldItems) {
					it->sendToFront();
					mNavItems.emplace_back(it);
					if(it == tvi) {
						auto theChillins = theModel.getChildren(childrenName);
						for(auto cit : theChillins) {
							addNavItem(parenty, tvi->mLayoutLPad + 20.0f, cit, "");
						}
					}

				}

				setTableData(theModel, childrenName);

				tvi->setExpanded(true);
			}


			runLayout();
		});
	}
	parenty->addChildPtr(tvi);
	mNavItems.emplace_back(tvi);


	if(childrenName.empty()) {
		for(auto it : theModel.getChildrenMap()) {
			if(it.second.size() == 1) {
				addNavItem(parenty, indent + 20.0f, it.second.front(), "");
			} else {
				addNavItem(parenty, indent + 20.0f, theModel, it.first);
			}
		}
	}

}

void TableView::setTableData(ds::model::DataModelRef theModel, const std::string& childrenName) {
	auto oldItems = mTableItems;
	mTableItems.clear();


	auto holder = getSprite("table_layout");
	if(!holder) return;

	for (auto it : theModel.getChildren(childrenName)){
		TableTableItem* tti = nullptr;
		if(!oldItems.empty()) {
			tti = oldItems.back();
			oldItems.pop_back();
		} else {
			tti = new TableTableItem(mGlobals);
		}

		holder->addChildPtr(tti);
		tti->setData(it);
		mTableItems.emplace_back(tti);
	}

	for (auto it : oldItems){
		it->release();
	}

	runLayout();
}

} // namespace downstream

