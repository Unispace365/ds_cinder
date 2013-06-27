#include "ds/physics/sprite_body.h"

#include <ds/app/app.h>
#include "ds/physics/world.h"
#include <ds/ui/sprite/sprite_engine.h>
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "Box2D/Dynamics/b2World.h"

namespace ds {
namespace physics {

namespace {

// Statically initialize the world class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::physics::World*		w = new ds::physics::World(e);
			if (!w) throw std::runtime_error("Can't create ds::physics::World");
			e.addService("physics", *w);
		});

	}

	void			doNothing() { }
};
Init				INIT;

}

/**
 * \class ds::physics::SpriteBody
 */
SpriteBody::SpriteBody(ds::ui::Sprite& owner)
	: mWorld(static_cast<ds::physics::World&>(owner.getEngine().getService("physics")))
	, mOwner(owner)
	, mBody(nullptr)
{
	// This shouldn't be necessary, but I just want to make sure the static is referenced.
	INIT.doNothing();
}

SpriteBody::~SpriteBody()
{
	destroy();
}

void SpriteBody::create()
{
	destroy();

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.userData = &mOwner;
	def.position = mWorld.Ci2BoxTranslation(mOwner.getPosition());
	mBody = mWorld.mWorld->CreateBody(&def);
	if (!mBody) return;

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(mOwner.getScaleWidth() / 2.0f * mWorld.getCi2BoxScale(), mOwner.getScaleHeight() / 2.0f * mWorld.getCi2BoxScale());

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = 1.0f;

	// Override the default friction.
	fixtureDef.friction = 1.0f; //mGlobals.mLayout.getFloat("node_friction", 0, 0.8f);

	// Add the shape to the body.
	mBody->CreateFixture(&fixtureDef);
}

void SpriteBody::destroy()
{
	if (mBody != nullptr) {
		mWorld.mWorld->DestroyBody(mBody);
		mBody = nullptr;
	}
}

void SpriteBody::setLinearVelocity(const float x, const float y)
{
	if (mBody != nullptr) {
		mBody->SetLinearVelocity(b2Vec2(x, y));
	}
}

} // namespace physics
} // namespace ds
