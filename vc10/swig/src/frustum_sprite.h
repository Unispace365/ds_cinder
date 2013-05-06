#include "ds/ui/sprite/sprite.h"
#include "cinder/Camera.h"
#include "cinder/Vector.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
	namespace ui {
	class FrustumSprite : public Sprite {
	public:
		FrustumSprite( SpriteEngine& engine, float fov, const cinder::Vec3f &center, float _near=1.0, float _far=1000.0, float w=0.0f, float h=0.0f );
		void drawClient( const ci::Matrix44f &trans, const DrawParams &drawParams );
		float mL, mR, mB, mT, mN, mF;

	private:
		cinder::CameraPersp mCamera;
	};
	} // namespace ui
} //namespace ds

/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */
