#pragma once
#ifndef DS_UI_LAYOUT_SMART_SCROLL_LIST
#define DS_UI_LAYOUT_SMART_SCROLL_LIST

#include "scroll_list.h"

namespace ds {
namespace ui {
class SmartLayout;

/**
* \class ds::ui::SmartScrollList
*		This is a scroll list that helps you add items easily using a content model
*		Each item in the scroll list will be a SmartLayout
*
*	NOTES:
*		- The itemLayout needs to be fixed height
*		- The item's root sprite needs to be enabled, but not the children
*		- Add a sprite named "highlight" to have it's visibility toggled when the row is touched
*		- Optionally add a scroll bar named "scroll_bar"
*
**/
class SmartScrollList : public ScrollList {
public:
	SmartScrollList(ds::ui::SpriteEngine& eng, const std::string& itemLayoutFile = "");

	/// Callback when an item in the list is tapped. 
	/// The SmartLayout is the item, the model is the ContentModelRef from that item
	void			setContentItemTappedCallback(std::function<void(SmartLayout*, ds::model::ContentModelRef theModel)> func) { mContentItemTapped = func; }

	/// Called everytime an item's model has been updated
	void			setContentItemUpdatedCallback(std::function<void(SmartLayout*)> func) { mContentItemUpdated = func; }

	/// Sets the contents, uses the list of direct children
	void			setContentList(ds::model::ContentModelRef parentModel);

	/// Sets the list of items to display
	void			setContentList(std::vector<ds::model::ContentModelRef> theContents);

	/// Sets the list of items to display and keeps the scroll percent
	void			setContentListMaintainPosition(ds::model::ContentModelRef parentModel);
	void			setContentListMaintainPosition(std::vector<ds::model::ContentModelRef> theContents);

	/// Sets the layout file for each item, MUST be fixed height
	void			setItemLayoutFile(const std::string& itemLayout);

protected:
	std::function<void(SmartLayout*)> mContentItemUpdated;
	std::map<int, ds::model::ContentModelRef> mContentMap;
	std::function<void(SmartLayout*, ds::model::ContentModelRef theModel)> mContentItemTapped;
};

}
}
#endif

