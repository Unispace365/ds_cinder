#pragma once
#ifndef LAYOUT_BUILDER_UI_SPRITE_CREATOR
#define LAYOUT_BUILDER_UI_SPRITE_CREATOR


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>


#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;
class TreeItem;

/**
* \class layout_builder::SpriteCreator
*			Buttons to make new sprites
*/
class SpriteCreator : public ds::ui::Sprite  {
public:
	SpriteCreator(Globals& g);

private:
	void								onAppEvent(const ds::Event&);
	void								createCreateButton(std::string typeName);

	void								animateOn();
	void								animateOff();
	void								layout();

	Globals&							mGlobals;
	ds::EventClient						mEventClient;
	ds::ui::LayoutSprite*				mLayout;
};

} // namespace layout_builder

#endif
