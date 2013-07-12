#include "private/world.h"

#include <ds/app/auto_update.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/physics/sprite_body.h>
#include "Box2D/Collision/Shapes/b2EdgeShape.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2World.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "Box2D/Dynamics/Joints/b2MouseJoint.h"

namespace ds {
namespace physics {

const ds::BitMask			PHYSICS_LOG = ds::Logger::newModule("physics");

// NOTE: Ideally we'd initialize the World as an Engine service in a static here.
// However, because I'm in a library statically linked to the main application, that
// initialization will get tossed, so it has to happen from a class that the app
// will reference. Currently it's happening in Body.

/**
 * \class ds::physics::World
 */
World::World(ds::ui::SpriteEngine& e)
	: ds::AutoUpdate(e)
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

	if (mSettings.getRectSize("bounds:fixed") > 0) {
		setBounds(mSettings.getRect("bounds:fixed"));
	} else if (mSettings.getRectSize("bounds:unit") > 0) {
		const ci::Rectf&		r = mSettings.getRect("bounds:unit");
		setBounds(ci::Rectf(r.x1 * e.getWorldWidth(), r.y1 * e.getWorldHeight(), r.x2 * e.getWorldWidth(), r.y2 * e.getWorldHeight()));
	}
}

void World::processTouchInfo(const SpriteBody& body, const ds::ui::TouchInfo& ti)
{
	if (ti.mPhase == ti.Added) {
		eraseTouch(ti.mFingerId);

		if (body.mBody) {
			b2MouseJointDef jointDef;
			jointDef.target = Ci2BoxTranslation(ti.mStartPoint);
			jointDef.bodyA = mGround;
			jointDef.bodyB = body.mBody;
			jointDef.maxForce = 10000.0f * body.mBody->GetMass();
			jointDef.dampingRatio = 1.0f;
			jointDef.frequencyHz = 25.0f;
			mTouchJoints[ti.mFingerId] = mWorld->CreateJoint(&jointDef);
		}
	} else if (ti.mPhase == ti.Moved) {
		b2MouseJoint*		j = getTouchJoint(ti.mFingerId);
		if (j) {
			j->SetTarget(Ci2BoxTranslation(ti.mCurrentGlobalPoint));
		}
	} else if (ti.mPhase == ti.Removed) {
		eraseTouch(ti.mFingerId);
	}
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

void World::update(const ds::UpdateParams& p)
{
	static const int	VELOCITY_ITERATIONS = 6;
	static const int	POSITION_ITERATIONS = 2;

	mWorld->Step(p.getDeltaTime(), VELOCITY_ITERATIONS, POSITION_ITERATIONS);

	// Update all objects
	for ( b2Body* b = mWorld->GetBodyList(); b; b = b->GetNext() )
	{
		if ( b->GetType() != b2_dynamicBody )
			continue;
		if ( b->IsAwake() )
		{
			ds::ui::Sprite*	sprite = reinterpret_cast<ds::ui::Sprite*>( b->GetUserData() );
			if ( sprite )
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

void World::setBounds(const ci::Rectf& f)
{
	if (f.x2 <= f.x1 || f.y2 <= f.y1) {
		DS_LOG_WARNING_M("World constructed on invalid bounds (" << f << ")", PHYSICS_LOG);
		return;
	}

	b2BodyDef		def;
	mBounds = mWorld->CreateBody(&def);
	if (!mBounds) throw std::runtime_error("ds::physics::World() can't create mBounds");

	b2EdgeShape		shape;
	b2FixtureDef	fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 0.1f;

	// left
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x1, f.y2, 0.0f)));
	mBounds->CreateFixture(&fixtureDef);
	// top
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y1, 0.0f)));
	mBounds->CreateFixture(&fixtureDef);
	// right
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x2, f.y1, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y2, 0.0f)));
	mBounds->CreateFixture(&fixtureDef);
	// bottom
	shape.Set(	Ci2BoxTranslation(ci::Vec3f(f.x1, f.y2, 0.0f)),
				Ci2BoxTranslation(ci::Vec3f(f.x2, f.y2, 0.0f)));
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
