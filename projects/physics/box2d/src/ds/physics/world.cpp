#include "ds/physics/world.h"

#include <ds/app/auto_update.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/sprite.h>
#include "Box2D/Dynamics/b2World.h"
#include "Box2D/Dynamics/b2Body.h"

namespace ds {
namespace physics {

// NOTE: Ideally we'd initialize the World as an Engine service in a static here.
// However, because I'm in a library statically linked to the main application, that
// initialization will get tossed, so it has to happen from a class that the app
// will reference. Currently it's happening in Body.

/**
 * \class ds::physics::World
 */
World::World(ds::ui::SpriteEngine& e)
	: ds::AutoUpdate(e)
	, mCi2BoxScale(0.02f)
{
	mWorld = std::move(std::unique_ptr<b2World>(new b2World(b2Vec2(0.0f, 0.0f))));
	if (mWorld.get() == nullptr) throw std::runtime_error("ds::physics::World() can't create b2World");
}

World::~World()
{
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

} // namespace physics
} // namespace ds
