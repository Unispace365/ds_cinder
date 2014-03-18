#include "ds/physics/body_builder.h"

#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "Box2D/Collision/Shapes/b2CircleShape.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "ds/physics/sprite_body.h"
#include "private/world.h"

namespace ds {
namespace physics {

/**
 * \class ds::physics::BodyBuilder
 */
BodyBuilder::BodyBuilder(const SpriteBody& b)
	: mDensity(1.0f)
	, mFriction(b.mWorld.getFriction())
	, mLinearDampening(b.mWorld.getLinearDampening())
	, mAngularDampening(b.mWorld.getAngularDampening())
	, mFixedRotation(b.mWorld.getFixedRotation())
	, mIsStaticBody(false)
{
	b2Filter		filter;
	mCategoryBits = filter.categoryBits;
	mMaskBits = filter.maskBits;
}

BodyBuilder::~BodyBuilder()
{
}

/**
 * \class ds::physics::BodyBuilderBox
 */
BodyBuilderBox::BodyBuilderBox(const SpriteBody& sb)
	: BodyBuilder(sb)
{
}

void BodyBuilderBox::createFixture(SpriteBody& body) const
{
	if (!body.mBody) return;

	// Define the dynamic body fixture.
	b2FixtureDef		fixtureDef;
	fixtureDef.density = mDensity;
	fixtureDef.friction = mFriction;
	fixtureDef.filter.categoryBits = mCategoryBits;
	fixtureDef.filter.maskBits = mMaskBits;

	b2PolygonShape	dynamicBox;
	const float32			w = mWidth / 2.0f * body.mWorld.getCi2BoxScale(),
							h = mHeight / 2.0f * body.mWorld.getCi2BoxScale();
#if 1
	dynamicBox.SetAsBox(w, h);
#else
	b2Vec2					center;
	center.x = w;
	center.y = 0.0f;
	dynamicBox.SetAsBox(w, h, center, 0.0f);
#endif
	fixtureDef.shape = &dynamicBox;
	body.mBody->CreateFixture(&fixtureDef);
}

/**
 * \class ds::physics::BodyBuilderCircle
 */
BodyBuilderCircle::BodyBuilderCircle(const SpriteBody& sb)
	: BodyBuilder(sb)
{
}

void BodyBuilderCircle::createFixture(SpriteBody& body) const
{
	if (!body.mBody) return;

	// Define the dynamic body fixture.
	b2FixtureDef		fixtureDef;
	fixtureDef.density = mDensity;
	fixtureDef.friction = mFriction;
	fixtureDef.filter.categoryBits = mCategoryBits;
	fixtureDef.filter.maskBits = mMaskBits;

	b2CircleShape	circle;
	circle.m_radius = mRadius * body.mWorld.getCi2BoxScale(); 
	fixtureDef.shape = &circle;
	body.mBody->CreateFixture(&fixtureDef);
}

} // namespace physics
} // namespace ds
