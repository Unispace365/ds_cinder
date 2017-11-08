#pragma once
#ifndef _PHYSICS_EXAMPLE_APP_UI_BOUNCY_VIEW_H_
#define _PHYSICS_EXAMPLE_APP_UI_BOUNCY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/physics/sprite_body.h>

#include "model/generated/story_model.h"

namespace physics {

class Globals;

/**
* \class physics::BouncyView
*			A sample view
*/
class BouncyView final : public ds::ui::Sprite {
public:
	BouncyView(Globals& g);
	~BouncyView();

private:

	void									animateOn();
	void									animateOff();
	Globals&								mGlobals;
	std::vector<ds::ui::Sprite*>			mCircles;
	std::vector<ds::physics::SpriteBody*>	mPhysicsBodies;

	void									onAppEvent(const ds::Event&);
	void									rebuildBouncies();
	void									clearBouncies();

	ds::EventClient							mEventClient;

};

} // namespace physics

#endif

#pragma once
