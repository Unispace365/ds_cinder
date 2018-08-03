#include "ds/physics/collision.h"

namespace ds {
namespace physics {

/**
 * \class Collision
 */
Collision::Collision()
	: mForce(0.0f)
	, mType(EMPTY)
	, mEdge(LEFT)
{
}

void Collision::setToWorldBounds(const Edge e)
{
	mType = WORLD_BOUNDS;
	mEdge = e;
}

void Collision::setToSprite(const ds::sprite_id_t id)
{
	mType = SPRITE;
	mSpriteId = id;
}

} // namespace physics
} // namespace ds
