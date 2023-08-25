#pragma once


#include <ds/app/event_client.h>
#include <ds/physics/sprite_body.h>
#include <ds/ui/sprite/sprite.h>

namespace physics {
class BouncyView final : public ds::ui::Sprite {
  public:
	BouncyView(ds::ui::SpriteEngine& g);
	~BouncyView();

  private:
	void								  animateOn();
	void								  animateOff();
	std::vector<ds::ui::Sprite*>		  mCircles;
	std::vector<ds::physics::SpriteBody*> mPhysicsBodies;

	void onAppEvent(const ds::Event&);
	void rebuildBouncies();
	void clearBouncies();

	ds::EventClient mEventClient;
};

} // namespace physics
