#pragma once
#ifndef LAYOUT_BUILDER_UI_TREE_ITEM
#define LAYOUT_BUILDER_UI_TREE_ITEM


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

namespace layout_builder {

class Globals;

/**
* \class layout_builder::TreeItem
*			A single item in a tree Hierarchy
*/
class TreeItem : public ds::ui::Sprite  {
public:
	TreeItem(Globals& g, ds::ui::Sprite* linkedItem); 
	TreeItem(Globals& g, const std::wstring& labelOne, const std::wstring& labelTwo);

	std::string							getPropertyName(){ if(mNameText) return mNameText->getTextAsString(); return ""; }
	ds::ui::Text*						getValueField(){ return mLabelTextTwo; }
	void								setValueText(const std::wstring& labelTwoText);
	void								setValueTappedCallback(std::function<void(TreeItem*)> tappedCallback);

private:
	void								initialize(const std::wstring& labelOne, const std::wstring& labelTwo);

	Globals&							mGlobals;
	ds::ui::Text*						mNameText;
	ds::ui::Text*						mLabelTextTwo;
	ds::ui::Sprite*						mLinkedSprite;

	std::function<void(TreeItem*)>		mValueTappedCallback;

};

} // namespace layout_builder

#endif
