#pragma once
#ifndef LAYOUT_BUILDER_UI_TREE_ITEM
#define LAYOUT_BUILDER_UI_TREE_ITEM


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

namespace layout_builder {

class Globals;

/**
* \class layout_builder::SpritePropertyItem
*			A single item in the sprite property view
*/
class SpritePropertyItem : public ds::ui::Sprite  {
public:
	SpritePropertyItem(Globals& g, const std::wstring& labelOne, const std::wstring& labelTwo);

	std::string							getPropertyName(){ if(mNameText) return mNameText->getTextAsString(); return ""; }
	ds::ui::Text*						getValueField(){ return mLabelTextTwo; }
	void								setValueText(const std::wstring& labelTwoText);
	void								setValueTappedCallback(std::function<void(SpritePropertyItem*)> tappedCallback);

private:
	void								initialize(const std::wstring& labelOne, const std::wstring& labelTwo);

	Globals&							mGlobals;
	ds::ui::Text*						mNameText;
	ds::ui::Text*						mLabelTextTwo;

	std::function<void(SpritePropertyItem*)>		mValueTappedCallback;

};

} // namespace layout_builder

#endif
