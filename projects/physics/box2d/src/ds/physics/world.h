#pragma once
#ifndef DS_PHYSICS_WORLD_H_
#define DS_PHYSICS_WORLD_H_

#include <cinder/Vector.h>
#include <ds/app/auto_update.h>
#include <ds/app/engine/engine_service.h>
class b2World;
struct b2Vec2;

namespace ds {
namespace physics {
class SpriteBody;

/**
 * \class ds::physics::World
 */
class World : public ds::EngineService
			, public ds::AutoUpdate {
public:
	World(ds::ui::SpriteEngine&);
	~World();

protected:
	virtual void				update(const ds::UpdateParams&);

private:
	float						getCi2BoxScale() const;
	ci::Vec3f					box2CiTranslation(const b2Vec2&);
	b2Vec2						Ci2BoxTranslation(const ci::Vec3f&);

	friend class ds::physics::SpriteBody;
	std::unique_ptr<b2World>	mWorld;
	const float					mCi2BoxScale;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_WORLD_H_
