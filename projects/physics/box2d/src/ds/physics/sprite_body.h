#pragma once
#ifndef DS_PHYSICS_SPRITEBODY_H_
#define DS_PHYSICS_SPRITEBODY_H_

class b2Body;

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;
}

namespace physics {
class BodyBuilder;
class World;

/**
 * \class ds::physics::SpriteBody
 * \brief This serves as the sprite entry point to the physics system.
 * Simply including this in a sprite subclass and calling create() is
 * enough to enable physics on a sprite.
 * By default this class will take over touch processing; if you want
 * to mix in your own behaviour, take it back.
 */
class SpriteBody {
public:
	SpriteBody(ds::ui::Sprite&);
	~SpriteBody();

	void					create(const BodyBuilder&);
	void					destroy();

	void					setLinearVelocity(const float x, const float y);

private:
	friend class BodyBuilder;
	friend class BodyBuilderBox;
	friend class BodyBuilderCircle;

	World&					mWorld;
	ds::ui::Sprite&			mOwner;
	b2Body*					mBody;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_SPRITEBODY_H_
