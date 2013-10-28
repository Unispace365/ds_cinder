#pragma once
#ifndef DS_PHYSICS_SPRITEBODY_H_
#define DS_PHYSICS_SPRITEBODY_H_

#include <vector>
#include <functional>
#include <cinder/Vector.h>

class b2Body;
class b2DistanceJoint;

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;
struct TouchInfo;
}

namespace physics {
class BodyBuilder;
class Collision;
class Touch;
class World;

/**
 * \class ds::physics::SpriteBody
 * \brief This serves as the sprite entry point to the physics system.
 * Simply including this in a sprite subclass and calling create() is
 * enough to enable physics on a sprite.
 * By default this class will take over touch processing; if you want
 * to mix in your own behaviour, take it back.
 */
class SpriteBody {
public:
	// Create a physics body on the sprite using the given world.
	// The default is the global world, which is only constructed
	// if the physics.xml setting "create_world_0" is true. Otherwise,
	// applications need to construct worlds as desired and reference them.
	SpriteBody(ds::ui::Sprite&, const int world_id = 0);
	~SpriteBody();

	bool					empty() const;
	void					create(const BodyBuilder&);
	void					createDistanceJoint(SpriteBody&, float length, float dampingRatio, float frequencyHz);
	void					destroy();
	void					setActive(bool flag);

	void					resizeDistanceJoint(SpriteBody& body, float length);

	void					processTouchInfo(ds::ui::Sprite*, const ds::ui::TouchInfo&);
	void					processTouchAdded(const ds::ui::TouchInfo&);
	void					processTouchMoved(const ds::ui::TouchInfo&);
	void					processTouchRemoved(const ds::ui::TouchInfo&);

	void					setPosition(const ci::Vec3f&);

	void					clearVelocity();
	void					setLinearVelocity(const float x, const float y);

	void					setRotation(const float degree);
	// Set a collision callback, called whenever this body
	// collides with another physics object.
	void					setCollisionCallback(const std::function<void(const Collision&)>& fn);

	// The sprite owner is resonsible for telling me when the
	// center changes. (Only necessary if the fixture is a box).
	void					onCenterChanged();

private:
	friend class BodyBuilder;
	friend class BodyBuilderBox;
	friend class BodyBuilderCircle;
	friend class World;
	friend class ds::physics::Touch;

	World&					mWorld;
	ds::ui::Sprite&			mSprite;
	b2Body*					mBody;
	std::vector<b2DistanceJoint*>		mJoints;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_SPRITEBODY_H_
