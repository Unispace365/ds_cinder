#include "private/touch.h"

#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/physics/sprite_body.h>
#include "private/world.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2World.h"
#include "Box2D/Dynamics/Joints/b2MouseJoint.h"

#include "cinder/CinderMath.h"
#include <cinder/Matrix44.h>

namespace ds {
namespace physics {

/**
 * \class ds::physics::Touch
 */
Touch::Touch(ds::physics::World& w)
		: mWorld(w) {
}

void Touch::processTouchAdded(const SpriteBody& body, const ds::ui::TouchInfo& ti) {	
	eraseTouch(ti.mFingerId);

	if (body.mBody) {
		b2MouseJointDef jointDef;
		jointDef.target = mWorld.Ci2BoxTranslation(ti.mStartPoint);
		jointDef.bodyA = mWorld.mGround;
		jointDef.bodyB = body.mBody;
		jointDef.maxForce = 10000.0f * body.mBody->GetMass();
		jointDef.dampingRatio = 1.0f;
		jointDef.frequencyHz = 25.0f;
		//Experimenting with Tree Node mousejoints (assuming colliding bodies):
		//jointDef.dampingRatio = 30.0f;
		//jointDef.frequencyHz = 40.0f;
		mTouchJoints[ti.mFingerId] = mWorld.mWorld->CreateJoint(&jointDef);
	}
}

namespace {
float			wrap(float in, float size) {
	while (in < 0) in += size;
	while (in > size) in -= size;
	return in;
}

ci::Vec3f	get_global_rotation(const ds::ui::Sprite& s) {
	ci::Vec3f				rotation(0.0f, 0.0f, 0.0f);
	const ds::ui::Sprite*	p = &s;
	while (p) {
		rotation += p->getRotation();
		p = p->getParent();
	}
	rotation.z = wrap(rotation.z, 360.0f);
	return rotation;
}
}

void Touch::processTouchMoved(const SpriteBody& body, const ds::ui::TouchInfo& ti) {
	b2MouseJoint*		j = getTouchJoint(ti.mFingerId);
	if (j) {
		ci::Vec3f		rotation(0.0f, 0.0f, 0.0f);
		if (body.mSprite.getParent()) {
			rotation = get_global_rotation(*(body.mSprite.getParent()));
		}
		ci::Vec3f		offset = ti.mCurrentGlobalPoint - ti.mStartPoint;
		ci::Vec3f		new_position(offset);
		const float		r = ci::toRadians(-rotation.z);
		new_position.rotateZ(r);

		j->SetTarget(mWorld.Ci2BoxTranslation(ti.mStartPoint + new_position));
	}
}

void Touch::processTouchRemoved(const SpriteBody& body, const ds::ui::TouchInfo& ti) {
	eraseTouch(ti.mFingerId);
}


void Touch::eraseTouch(const int fingerId)
{
	if (mTouchJoints.empty()) return;

	auto it = mTouchJoints.find(fingerId);
	if (it != mTouchJoints.end()) {
		b2MouseJoint*	j = getTouchJointFromPtr(it->second);
		if (j) mWorld.mWorld->DestroyJoint(j);
		mTouchJoints.erase(it);
	}
}

b2MouseJoint* Touch::getTouchJoint(const int fingerId)
{
	if (mTouchJoints.empty()) return nullptr;

	// Be safe about the touch joints -- they can get destroyed via things
	// like destroying bodies (I think... if not, might rethink this), so
	// look them up.
	auto it = mTouchJoints.find(fingerId);
	if (it != mTouchJoints.end()) {
		return getTouchJointFromPtr(it->second);
	}
	return nullptr;
}

b2MouseJoint* Touch::getTouchJointFromPtr(const void* ptr)
{
	if (!ptr) return nullptr;

	// Be safe about the touch joints -- they can get destroyed via things
	// like destroying bodies (I think... if not, might rethink this), so
	// look them up.
	b2Joint*		j = mWorld.mWorld->GetJointList();
	while (j) {
		if (j == ptr) return dynamic_cast<b2MouseJoint*>(j);
		j = j->GetNext();
	}
	return nullptr;
}

} // namespace physics
} // namespace ds
