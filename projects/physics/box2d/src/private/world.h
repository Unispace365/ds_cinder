#pragma once
#ifndef DS_PHYSICS_PRIVATE_WORLD_H_
#define DS_PHYSICS_PRIVATE_WORLD_H_

#include <unordered_map>
#include <cinder/Vector.h>
#include <ds/app/auto_update.h>
#include <ds/app/engine/engine_service.h>
#include <ds/cfg/settings.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/util/bit_mask.h>
#include "private/contact_listener.h"
class b2Body;
class b2MouseJoint;
class b2World;
struct b2Vec2;

namespace ds {
namespace physics {
class SpriteBody;

extern const ds::BitMask		PHYSICS_LOG;

/**
 * \class ds::physics::World
 */
class World : public ds::EngineService
			, public ds::AutoUpdate {
public:
	World(ds::ui::SpriteEngine&);

	void							createDistanceJoint(const SpriteBody&, const SpriteBody&, float length);

	void							processTouchAdded(const SpriteBody&, const ds::ui::TouchInfo&);
	void							processTouchMoved(const SpriteBody&, const ds::ui::TouchInfo&);
	void							processTouchRemoved(const SpriteBody&, const ds::ui::TouchInfo&);

	float							getCi2BoxScale() const;

	float							getFriction() const;
	float							getLinearDampening() const;
	float							getAngularDampening() const;
	bool							getFixedRotation() const;

	void							setCollisionCallback(const ds::ui::Sprite&, const std::function<void(const Collision&)>& fn);
	// Fill out the collision object, if it applies to me, otherwise return false
	bool							makeCollision(const b2Fixture&, Collision&) const;

	ci::Vec3f						box2CiTranslation(const b2Vec2&);
	b2Vec2							Ci2BoxTranslation(const ci::Vec3f&);

protected:
	virtual void					update(const ds::UpdateParams&);

private:
	void							setBounds(const ci::Rectf&, const float restitution);

	// Answer a joint for a finger ID
	void							eraseTouch(const int fingerId);
	b2MouseJoint*					getTouchJoint(const int fingerId);
	b2MouseJoint*					getTouchJointFromPtr(const void*);

	friend class ds::physics::SpriteBody;

	std::unique_ptr<b2World>		mWorld;
	ContactListener					mContactListener;
	// Only register the listener with the world if there's someone listening;
	// otherwise, this can incur a bit of overhead.
	bool							mContactListenerRegistered;
	b2Body*							mGround;
	b2Body*							mBounds;

	const float						mCi2BoxScale;

	ds::cfg::Settings				mSettings;
	// Cached from the settings
	float							mFriction,
									mLinearDampening,
									mAngularDampening;
	bool							mFixedRotation;

	// Store pointers to joints created with touches.
	std::unordered_map<int, void*>	mTouchJoints;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_PRIVATE_WORLD_H_
