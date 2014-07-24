#include "debug_draw.h"

#include "Box2D/Dynamics/b2World.h"

namespace ds {
namespace physics {

/**
 * \class ds::physics::DebugDraw
 */
DebugDraw::DebugDraw(ds::ui::SpriteEngine& e, b2World& w)
		: ds::AutoDraw(e)
		, mWorld(w) {
	mWorld.SetDebugDraw(this);
	SetFlags( b2Draw::e_shapeBit | b2Draw::e_jointBit );
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
#if _DEBUG
	std::cout << "DebugDraw::DrawSolidPolygon()" << std::endl;
#endif
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
}

void DebugDraw::DrawTransform(const b2Transform& xf) {
}

void DebugDraw::drawClient(const ci::Matrix44f& t, const DrawParams& p) {
	mWorld.DrawDebugData();
}


} // namespace physics
} // namespace ds
