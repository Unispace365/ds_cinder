#pragma once
#ifndef DS_UI_BACKGROUND_COMPONENT_MENU_ITEM
#define DS_UI_BACKGROUND_COMPONENT_MENU_ITEM

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>
#include <string>

#include "ds/ui/menu/touch_menu.h"

namespace ds{
namespace ui{

class MenuItem : public ds::ui::Sprite {
public:
	MenuItem(ds::ui::SpriteEngine& enginey, const ds::ui::TouchMenu::MenuItemModel itemModel, const ds::ui::TouchMenu::TouchMenuConfig config);


	// 0 = left, 1 = top, 2 = right, 3 = bottom
	void								animateOn(int direction);
	void								animateOff(int direction);

	void								highlight();
	void								unhighlight();
	bool								getHighlited(){ return mHighlighted; }
	ds::ui::TouchMenu::MenuItemModel&	getModel(){ return mMenuItemModel; }

	void								setAngle(const float rot){ mRotation = rot; }
	float								getAngle(){ return mRotation; }

private:
	bool								mHighlighted;
	bool								mActive;

	ds::ui::Text*						mTitle;

	ds::ui::Sprite*						mClipper;
	ds::ui::Sprite*						mClippy;
	ds::ui::Sprite*						mIcon;
	ds::ui::Sprite*						mIconGlow;
	ds::ui::Sprite*						mLines;

	ds::ui::TouchMenu::MenuItemModel	mMenuItemModel;
	ds::ui::TouchMenu::TouchMenuConfig	mMenuConfig;

	float								mRotation;
};
}
}

#endif