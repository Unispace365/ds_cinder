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

	ds::ui::Sprite*						getLinkedSprite(){ return mLinkedSprite; }
	std::string							getPropertyName(){ if(mNameText) return mNameText->getTextAsString(); return ""; }
	ds::ui::Text*						getValueField(){ return mLabelTextTwo; }
	void								setValueText(const std::wstring& labelTwoText);

private:

	Globals&							mGlobals;
	ds::ui::Text*						mNameText;
	ds::ui::Text*						mLabelTextTwo;
	ds::ui::Sprite*						mLinkedSprite;


};

} // namespace layout_builder

#endif


