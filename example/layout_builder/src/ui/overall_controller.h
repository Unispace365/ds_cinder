#pragma once
#ifndef LAYOUT_BUILDER_UI_OVERALL_CONTROLLER
#define LAYOUT_BUILDER_UI_OVERALL_CONTROLLER


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>


#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;

/**
* \class layout_builder::OverallController
*			control overall
*/
class OverallController : public ds::ui::Sprite  {
public:
	OverallController(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::LayoutSprite*				mController;


};

} // namespace layout_builder

#endif
