#include "private/world.h"

#include <ds/app/auto_update.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/physics/collision.h>
#include <ds/physics/sprite_body.h>
#include "Box2D/Collision/Shapes/b2EdgeShape.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2World.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "Box2D/Dynamics/Joints/b2DistanceJoint.h"
#include "Box2D/Dynamics/Joints/b2MouseJoint.h"

namespace ds {
namespace physics {

const ds::BitMask			PHYSICS_LOG = ds::Logger::newModule("physics");

namespace {
// All this just to keep track of the bound edges
int							BOUNDS_LEFT = 1;
int							BOUNDS_TOP = 2;
int							BOUNDS_RIGHT = 3;
int							BOUNDS_BOTTOM = 4;
void*						BOUNDS_LEFT_PTR = reinterpret_cast<void*>(&BOUNDS_LEFT);
void*						BOUNDS_TOP_PTR = reinterpret_cast<void*>(&BOUNDS_TOP);
void*						BOUNDS_RIGHT_PTR = reinterpret_cast<void*>(&BOUNDS_RIGHT);
void*						BOUNDS_BOTTOM_PTR = reinterpret_cast<void*>(&BOUNDS_BOTTOM);
}

// NOTE: Ideally we'd initialize the World as an Engine service in a static here.
// However, because I'm in a library statically linked to the main application, that
// initialization will get tossed, so it has to happen from a class that the app
// will reference. Currently it's happening in Body.

/**
 * \class ds::physics::World
 */
World::World(ds::ui::SpriteEngine& e)
	: ds::AutoUpdate(e)
	, mContactListener(*this)
	, mContactListenerRegistered(false)
	, mGround(nullptr)
	, mBounds(nullptr)
	, mCi2BoxScale(0.02f)
	, mFriction(0.5f)
	, mLinearDampening(0.0f)
	, mAngularDampening(0.0f)
	, mFixedRotation(true)
{
	mWorld = std::move(std::unique_ptr<b2World>(new b2World(b2Vec2(0.0f, 0.0f))));
	if (mWorld.get() == nullptr) throw std::runtime_error("ds::physics::World() can't create b2World");
	b2BodyDef		def;
	mGround = mWorld->CreateBody(&def);
	if (!mGround) throw std::runtime_error("ds::physics::World() can't create mGround");

	ds::Environment::loadSettings("physics.xml", mSettings);
	mFriction = mSettings.getFloat("friction", 0, mFriction);
	mLinearDampening = mSettings.getFloat("dampening:linear", 0, mLinearDampening);
	mAngularDampening = mSettings.getFloat("dampening:angular", 0, mAngularDampening);
	mFixedRotation = mSettings.getBool("rotation:fixed", 0, mFixedRotation);

	// Slightly complicated, but flexible: Bounds can be either fixed or unit,
	// or a combination of both, which applies the fixed as an offset.
	if (mSettings.getRectSize("bounds:fixed") > 0 && mSettings.getRectSize("bounds:unit") > 0) {
		const ci::Rectf&		unit = mSettings.getRect("bounds:unit");
		const ci::Rectf&		fixed = mSettings.getRect("bounds:fixed");
		ci::Rectf				r(	(unit.x1 * e.getWorldWidth()) + fixed.x1, (unit.y1 * e.getWorldHeight()) + fixed.y1,
									(unit.x2 * e.getWorldWidth()) + fixed.x2, (unit.y2 * e.getWorldHeight()) + fixed.y2);
		setBounds(r, mSettings.getFloat("bounds:restitution", 0, 1.0f));
	} else if (mSettings.getRectSize("bounds:fixed") > 0) {
		setBounds(	mSettings.getRect("bounds:fixed"),
					mSettings.getFloat("bounds:restitution", 0, 1.0f));
	} else if (mSettings.getRectSize("bounds:unit") > 0) {
		const ci::Rectf&		r = mSettings.getRect("bounds:unit");
		setBounds(	ci::Rectf(r.x1 * e.getWorldWidth(), r.y1 * e.getWorldHeight(), r.x2 * e.getWorldWidth(), r.y2 * e.getWorldHeight()),
					mSettings.getFloat("bounds:restitution", 0, 1.0f));
	}
}

void World::createDistanceJoint(const SpriteBody& body1, const SpriteBody& body2, float length, float dampingRatio, float frequencyHz) {
	
	if (body1.mBody && body2.mBody) {
		b2DistanceJointDef jointDef;
		jointDef.bodyA = body1.mBody;
		jointDef.bodyB = body2.mBody;
		jointDef.localAnchorA = Ci2BoxTranslation(body1.mSprite.getCenter());
		jointDef.localAnchorB = Ci2BoxTranslation(body2.mSprite.getCenter());
		// First try settings:
		//jointDef.dampingRatio = 8.0f;
		//jointDef.frequencyHz = 3.0f;
		// Settings that work well if you don't have colliding bodies:
		//jointDef.dampingRatio = 50.0f;
		//jointDef.frequencyHz = 10.0f;
		// Trying to find settings that work well with colliding bodies:
		//jointDef.dampingRatio = 0.95f;
		//jointDef.frequencyHz = 2.0f;
		// Trying out yet more settings for colliding bodies. The previous ones were alright, but not perfect:
		//jointDef.dampingRatio = 0.7f;
		//jointDef.frequencyHz = 1.5f;
		// DNA Wall's settings:
		//jointDef.dampingRatio = 1.0f;
		//jointDef.frequencyHz = 5.0f;
		
		jointDef.dampingRatio = dampingRatio;
		jointDef.frequencyHz = frequencyHz;
		
		jointDef.collideConnected = true;
		jointDef.length = getCi2BoxScale()*(length);
		b2DistanceJoint* joint = (b2DistanceJoint*) mWorld->CreateJoint(&jointDef);

		//DS_LOG_INFO_M("Joint anchors a=(" << joint->GetAnchorA().x << ", " << joint->GetAnchorA().y << ") b=(" << joint->GetAnchorB().x << ", " << joint->GetAnchorB().y << ")", PHYSICS_LOG);

		mDistanceJoints.insert(mDistanceJoints.end(), joint);
	}
}

void World::resizeDistanceJoint(const SpriteBody& body1, const SpriteBody& body2, float length) {
	for(auto it  = mDistanceJoints.begin(); it != mDistanceJoints.end(); ++it) {
		b2DistanceJoint* joint  = *it;
		if (joint->GetBodyA() == body1.mBody && joint->GetBodyB() == body2.mBody
			|| joint->GetBodyB() == body1.mBody && joint->GetBodyA() == body2.mBody) {
				joint->SetLength(length*getCi2BoxScale());
		}
	}
}

void World::processTouchAdded(const SpriteBody& body, const ds::ui::TouchInfo& ti) {	
	eraseTouch(ti.mFingerId);

	if (body.mBody) {
		b2MouseJointDef jointDef;
		jointDef.target = Ci2BoxTranslation(ti.mStartPoint);
		jointDef.bodyA = mGround;
		jointDef.bodyB = body.mBody;
		jointDef.maxForce = 10000.0f * body.mBody->GetMass();
		jointDef.dampingRatio = 1.0f;
		jointDef.frequencyHz = 25.0f;
		//Experimenting with Tree Node mousejoints (assuming colliding bodies):
		//jointDef.dampingRatio = 30.0f;
		//jointDef.frequencyHz = 40.0f;
		mTouchJoints[ti.mFingerId] = mWorld->CreateJoint(&jointDef);
	}
}

void World::processTouchMoved(const SpriteBody& body, const ds::ui::TouchInfo& ti) {
	b2MouseJoint*		j = getTouchJoint(ti.mFingerId);
	if (j) {
		j->SetTarget(Ci2BoxTranslation(ti.mCurrentGlobalPoint));
	}
}

void World::processTouchRemoved(const SpriteBody& body, const ds::ui::TouchInfo& ti) {
	eraseTouch(ti.mFingerId);
}

float World::getFriction() const
{
	return mFriction;
}

float World::getLinearDampening() const
{
	return mLinearDampening;
}

float World::getAngularDampening() const
{
	return mAngularDampening;
}

bool World::getFixedRotation() const
{
	return mFixedRotation;
}

void World::setCollisionCallback(const ds::ui::Sprite& s, const std::function<void(const Collision&)>& fn)
{
	mContactListener.setCollisionCallback(s, fn);
	if (!mContactListenerRegistered) {
		mContactListenerRegistered = true;
		mWorld->SetContactListener(&mContactListener);
	}
}

bool World::makeCollision(const b2Fixture& fix, Collision& c) const
{
	if (!mBounds) return false;
	if (fix.GetBody() != mBounds) return false;

	if (fix.GetUserData() == BOUNDS_LEFT_PTR) c.setToWorldBounds(Collision::LEFT);
	else if (fix.GetUserData() == BOUNDS_TOP_PTR) c.setToWorldBounds(Collision::TOP);
	else if (fix.GetUserData() == BOUNDS_RIGHT_PTR) c.setToWorldBounds(Collision::RIGHT);
	else if (fix.GetUserData() == BOUNDS_BOTTOM_PTR) c.setToWorldBounds(Collision::BOTTOM);
	else return false;

	return true;
}

void World::update(const ds::UpdateParams& p)
{
	static const int	VELOCITY_ITERATIONS = 6;
	static const int	POSITION_ITERATIONS = 2;

	mContactListener.clear();
	mWorld->Step(p.getDeltaTime(), VELOCITY_ITERATIONS, POSITION_ITERATIONS);

	// Update all objects
	for ( b2Body* b = mWorld->GetBodyList(); b; b = b->GetNext() )
	{
		if ( b->GetType() != b2_dynamicBody )
			continue;
		if ( b->IsAwake() )
		{
			ds::ui::Sprite*	sprite = reinterpret_cast<ds::ui::Sprite*>( b->GetUserData() );
			if (sprite)
			{
				auto pos = box2CiTranslation(b->GetPosition());
				sprite->setPosition(pos);
				sprite->setRotation(b->GetAngle() * 180.0f / 3.1415926f);
			}
		}
	}
#if 0
	for ( b2Joint *j = mWorld->GetJointList(); j; j = j->GetNext() )
	{
		if ( j->IsActive() )
		{
			JointView *joint_view = reinterpret_cast<JointView *>( j->GetUserData() );
			if ( joint_view )
			{
				joint_view->updateJoint();
			}
		}
	}
	///// Sort the nodes
	//mNodeWindow->getSpriteListRef().sort( [](BaseSprite *a, BaseSprite *b)
	//{
	//	return ( a->getPosition().y < b->getPosition().y );
	//});

	//auto slist = mNodeWindow->getSpriteList();
	//for ( auto i = slist.begin(), tend = slist.end(); i != tend; i++ ) {
	//	(*i)->sendToFront();
	//}
#endif

	mContactListener.report();
}


float World::getCi2BoxScale() const
{
	return mCi2BoxScale;
}

ci::Vec3f World::box2CiTranslation(const b2Vec2 &vec)
{
	return ci::Vec3f(vec.x / mCi2BoxScale, vec.y / mCi2BoxScale, 0.0f);
}

b2Vec2 World::Ci2BoxTranslation(const ci::Vec3f &vec)
{
	return b2Vec2(vec.x * mCi2BoxScale, vec.y * mCi2BoxScale);
}

void World::setBounds(const ci::Rectf& f, const float restitution)
{
	if (f.x2 <= f.x1 || f.y2 <= f.y1) {
		DS_LOG_WARNING_M("World constructed on invalid bounds (" << f << ")", PHYSICS_LOG);
		return;
	}

	b2BodyDef		def;
	def.type = b2_staticBody;
	def.fixedRotation = true;
	mBounds = mWorld->CreateBody(&def);
	if (!mBounds) throw std::runtime_error("ds::physics::World() can't create mBounds");

	b2EdgeShape		shape;
	b2FixtureDef	fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = mFriction;
	fixtureDef.restitution = restitution;

	// left
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x1, f.y2, 0.0f)));
	fixtureDef.userData = BOUNDS_LEFT_PTR;
	mBounds->CreateFixture(&fixtureDef);
	// top
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y1, 0.0f)));
	fixtureDef.userData = BOUNDS_TOP_PTR;
	mBounds->CreateFixture(&fixtureDef);
	// right
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x2, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y2, 0.0f)));
	fixtureDef.userData = BOUNDS_RIGHT_PTR;
	mBounds->CreateFixture(&fixtureDef);
	// bottom
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y2, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y2, 0.0f)));
	fixtureDef.userData = BOUNDS_BOTTOM_PTR;
	mBounds->CreateFixture(&fixtureDef);
}

void World::eraseTouch(const int fingerId)
{
	if (mTouchJoints.empty()) return;

	auto it = mTouchJoints.find(fingerId);
	if (it != mTouchJoints.end()) {
		b2MouseJoint*	j = getTouchJointFromPtr(it->second);
		if (j) mWorld->DestroyJoint(j);
		mTouchJoints.erase(it);
	}
}

b2MouseJoint* World::getTouchJoint(const int fingerId)
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

b2MouseJoint* World::getTouchJointFromPtr(const void* ptr)
{
	if (!ptr) return nullptr;

	// Be safe about the touch joints -- they can get destroyed via things
	// like destroying bodies (I think... if not, might rethink this), so
	// look them up.
	b2Joint*		j = mWorld->GetJointList();
	while (j) {
		if (j == ptr) return dynamic_cast<b2MouseJoint*>(j);
		j = j->GetNext();
	}
	return nullptr;
}

} // namespace physics
} // namespace ds
