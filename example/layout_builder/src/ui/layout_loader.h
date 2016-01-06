#pragma once
#ifndef LAYOUT_BUILDER_UI_LAYOUT_LOADER
#define LAYOUT_BUILDER_UI_LAYOUT_LOADER


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>


#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;

/**
* \class nwm::LayoutLoader
*			Load a layout
*/
class LayoutLoader : public ds::ui::Sprite  {
public:
	LayoutLoader(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								loadLayout(const std::string& layoutLocation);
	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::LayoutSprite*				mLayout;
	std::string							mLayoutLocation;


};

} // namespace layout_builder

#endif
