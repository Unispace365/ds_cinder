#pragma once
#ifndef DS_PHYSICS_PRIVATE_CONTACTLISTENER_H_
#define DS_PHYSICS_PRIVATE_CONTACTLISTENER_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "Box2D/Dynamics/b2WorldCallbacks.h"
#include "contact_key.h"

namespace ds {
namespace ui {
class Sprite;
}

namespace physics {
class Collision;
class World;

/**
 * \class ds::physics::ContactListener
 * \brief Report collisions between bodies.
 */
class ContactListener : public b2ContactListener
{
public:
	ContactListener(World&);

	virtual void		PostSolve(b2Contact*, const b2ContactImpulse*);

	// Either add or remove, depending on if the function is valid.
	void				setCollisionCallback(const ds::ui::Sprite&, const std::function<void(const Collision&)>& fn);

	// In each update, first clear me, then report after running
	// the physics simulation.
	void				clear();
	void				report();

private:
	void				collide(const b2Fixture* a, const b2Fixture* b, const b2ContactImpulse&);
	void				makeCollision(const ContactKey&, Collision&) const;

	World&				mWorld;

	// The collection of all sprites registered with me.
	std::unordered_map<const ds::ui::Sprite*, std::function<void(const Collision&)>>
						mRegistered;
	// The list of sprites to report on in the current update.
	std::unordered_set<ContactKey>
						mReport;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_PRIVATE_WORLD_H_
