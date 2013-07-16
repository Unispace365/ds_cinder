#pragma once
#ifndef DS_PHYSICS_PRIVATE_CONTACTLISTENER_H_
#define DS_PHYSICS_PRIVATE_CONTACTLISTENER_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "Box2D/Dynamics/b2WorldCallbacks.h"

namespace ds {
namespace ui {
class Sprite;
}

namespace physics {

/**
 * \class ds::physics::ContactListener
 * \brief Report collisions between bodies.
 */
class ContactListener : public b2ContactListener
{
public:
	ContactListener();

	virtual void		BeginContact(b2Contact*);

	// Either add or remove, depending on if the function is valid.
	void				setCollisionCallback(const ds::ui::Sprite&, const std::function<void(void)>& fn);

	// In each update, first clear me, then report after running
	// the physics simulation.
	void				clear();
	void				report();

private:
	void				collide(const b2Fixture*);

	// The collection of all sprites registered with me.
	std::unordered_map<const ds::ui::Sprite*, std::function<void(void)>>
						mRegistered;
	// The list of sprites to report on in the current update.
	std::unordered_set<const ds::ui::Sprite*>
						mReport;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_PRIVATE_WORLD_H_
