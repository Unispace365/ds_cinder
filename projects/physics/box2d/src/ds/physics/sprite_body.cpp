#include "ds/physics/sprite_body.h"

#include <ds/app/app.h>
#include "ds/physics/body_builder.h"
#include "private/world.h"
#include <ds/ui/sprite/sprite_engine.h>
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
	: mWorld(owner.getEngine().getService<ds::physics::World>("physics"))
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

void SpriteBody::create(const BodyBuilder& b)
{
	destroy();

	b2BodyDef			def;
	def.type = b2_dynamicBody;
	def.userData = &mOwner;
	def.position = mWorld.Ci2BoxTranslation(mOwner.getPosition());
	def.linearDamping = b.mLinearDampening;
	def.angularDamping = b.mAngularDampening;

	mBody = mWorld.mWorld->CreateBody(&def);
	if (!mBody) return;

	b.createFixture(*this);
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
