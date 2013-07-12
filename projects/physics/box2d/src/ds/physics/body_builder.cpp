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
//	dynamicBox.SetAsBox(body.mOwner.getScaleWidth() / 2.0f * mWorld.getCi2BoxScale(), body.mOwner.getScaleHeight() / 2.0f * mWorld.getCi2BoxScale());
	dynamicBox.SetAsBox(mWidth / 2.0f * body.mWorld.getCi2BoxScale(), mHeight / 2.0f * body.mWorld.getCi2BoxScale());
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
	circle.m_radius = mRadius; 
	fixtureDef.shape = &circle;
	body.mBody->CreateFixture(&fixtureDef);
}

} // namespace physics
} // namespace ds
