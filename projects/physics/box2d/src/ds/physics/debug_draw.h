#pragma once

#include <cinder/Matrix.h>

#include <ds/app/auto_draw.h>

#include "Box2D/Common/b2Draw.h"
class b2World;

namespace ds { namespace physics {

	class World;

	/**
	 * \class ds::physics::DebugDraw
	 */
	class DebugDraw : public b2Draw, public ds::AutoDraw {
	  public:
		DebugDraw(ds::ui::SpriteEngine&, b2World&, ds::physics::World&);
		~DebugDraw() override;

		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
		void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;
		void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
		void DrawTransform(const b2Transform& xf) override;

	  protected:
		void drawClient(const ci::mat4&, const DrawParams&) override;

	  private:
		ds::physics::World& mPhysicsWorld;
		b2World&			mB2World;
	};

}} // namespace ds::physics
