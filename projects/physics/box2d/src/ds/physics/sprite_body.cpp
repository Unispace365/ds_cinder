#include "ds/physics/sprite_body.h"

#include <ds/app/app.h>
#include "ds/physics/body_builder.h"
#include "private/service.h"
#include "private/world.h"
#include <ds/math/math_defs.h>
#include <ds/ui/sprite/sprite_engine.h>
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "Box2D/Dynamics/b2World.h"
#include "Box2D/Dynamics/Joints/b2Joint.h"
#include "Box2D/Dynamics/Joints/b2DistanceJoint.h"

namespace ds {
namespace physics {

const int		WORLD_CATEGORY_BIT = 1;

namespace {

// Statically initialize the service class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::physics::Service*		s = new ds::physics::Service(e);
			if (!s) throw std::runtime_error("Can't create ds::physics::Service");
			e.addService(SERVICE_NAME, *s);
		});

	}

	void			doNothing() { }
};
Init				INIT;

}

/**
 * \class ds::physics::SpriteBody
 */
SpriteBody::SpriteBody(ds::ui::Sprite& s, const int world_id)
		: mWorld(s.getEngine().getService<ds::physics::Service>(SERVICE_NAME).getWorld(world_id))
		, mSprite(s)
		, mBody(nullptr) {
	// This shouldn't be necessary, but I just want to make sure the static is referenced.
	INIT.doNothing();

	s.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	s.setProcessTouchCallback([this](ds::ui::Sprite* s, const ds::ui::TouchInfo& ti) { this->processTouchInfo(s, ti); });
}

SpriteBody::~SpriteBody() {
	mWorld.setCollisionCallback(mSprite,  nullptr);
	destroy();
}

bool SpriteBody::empty() const {
	return mBody == nullptr;
}

void SpriteBody::create(const BodyBuilder& b) {
	destroy();

	b2BodyDef			def;
	
	if (b.mIsStaticBody) {
		def.type = b2_staticBody;
	} else {
		def.type = b2_dynamicBody;
	}

	def.userData = &mSprite;
	def.position = mWorld.Ci2BoxTranslation(mSprite.getPosition());
	def.linearDamping = b.mLinearDampening;
	def.angularDamping = b.mAngularDampening;
	def.fixedRotation = b.mFixedRotation;

	mBody = mWorld.mWorld->CreateBody(&def);
	if (!mBody) return;

	b.createFixture(*this);
}

void SpriteBody::createDistanceJoint(SpriteBody& body, float length, float dampingRatio, float frequencyHz) {
	mWorld.createDistanceJoint(*this, body, length, dampingRatio, frequencyHz);
}
 
void SpriteBody::destroy() {
	if (mBody == nullptr) return;

	// Destroying a body also destroys all joints associated with that body.
	mWorld.mWorld->DestroyBody(mBody);
	mBody = nullptr;
}

void SpriteBody::setActive(bool flag) {
	if (!mBody) return;
	// Setting a body as inactive also sets all associated joints as inactive, but does not delete them from the world.
	mBody->SetActive(flag);
}

void SpriteBody::enableCollisions(const bool on) {
	if (!mBody) return;

	const bool		sensor = !on;
	b2Fixture*		fix = mBody->GetFixtureList();
	while (fix) {
		fix->SetSensor(sensor);
		fix = fix->GetNext();
	}
}

void SpriteBody::resizeDistanceJoint(SpriteBody& body, float length) {

	mWorld.resizeDistanceJoint(*this, body, length);
}

void SpriteBody::processTouchInfo(ds::ui::Sprite*, const ds::ui::TouchInfo& ti) {
	if (mBody == nullptr) return;

	if (ti.mPhase == ti.Added) mWorld.processTouchAdded(*this, ti);
	else if (ti.mPhase == ti.Moved) mWorld.processTouchMoved(*this, ti);
	else if (ti.mPhase == ti.Removed) mWorld.processTouchRemoved(*this, ti);
}

void SpriteBody::processTouchAdded(const ds::ui::TouchInfo& ti) {
	mWorld.processTouchAdded(*this, ti);
}

void SpriteBody::processTouchMoved(const ds::ui::TouchInfo& ti) {
	mWorld.processTouchMoved(*this, ti);
}

void SpriteBody::processTouchRemoved(const ds::ui::TouchInfo& ti) {
	mWorld.processTouchRemoved(*this, ti);
}

void SpriteBody::setPosition(const ci::Vec3f& pos) {
	if (mBody == nullptr) return;

	const b2Vec2		boxpos = mWorld.Ci2BoxTranslation(pos);
	mBody->SetTransform(boxpos, mBody->GetAngle());
}

void SpriteBody::clearVelocity() {
	if (mBody == nullptr) return;

	b2Vec2			zeroVec;
	zeroVec.SetZero();
	mBody->SetLinearVelocity(zeroVec);
	mBody->SetAngularVelocity(0);
}

void SpriteBody::setLinearVelocity(const float x, const float y) {
	if (mBody != nullptr) {
		mBody->SetLinearVelocity(b2Vec2(x, y));
	}
}

void SpriteBody::applyForceToCenter(const float x, const float y) {
	if (mBody != nullptr) {
		mBody->ApplyForceToCenter(b2Vec2(x, y));
	}
}

void SpriteBody::setRotation(const float degree) {
	if (mBody == nullptr) return;

	const float		angle = degree * ds::math::DEGREE2RADIAN;
	mBody->SetTransform(mBody->GetPosition(), angle);
	if (!mBody->IsAwake()) {
		// You'd think setting the transform would wake up the body,
		// but nope.
		mBody->SetAwake(true);
	}
}

float SpriteBody::getRotation() const {
	if (mBody == nullptr) return 0.0f;
	return mBody->GetAngle() * ds::math::RADIAN2DEGREE;
}

void SpriteBody::setCollisionCallback(const std::function<void(const Collision&)>& fn) {
	mWorld.setCollisionCallback(mSprite, fn);
}

void SpriteBody::onCenterChanged() {
	if (mBody == nullptr) return;

	// Currently there should only be 1 fixture.
	b2Fixture*				fix = mBody->GetFixtureList();
	while (fix) {
		b2PolygonShape*		poly = dynamic_cast<b2PolygonShape*>(fix->GetShape());
		if (poly) {
			const float32	w = mSprite.getWidth() / 2.0f * mWorld.getCi2BoxScale(),
							h = mSprite.getHeight() / 2.0f * mWorld.getCi2BoxScale();
			// Convert the sprite center into a box2d center.
			const ci::Vec2f	cen((mSprite.getCenter().x * 2) - 1.0f,
								(mSprite.getCenter().y * 2) - 1.0f);
			b2Vec2			box_cen;
			box_cen.x = cen.x * -w;
			box_cen.y = cen.y * -h;
			poly->SetAsBox(w, h, box_cen, 0.0f);

			return;
		}
		fix = fix->GetNext();
	}
}

} // namespace physics
} // namespace ds
