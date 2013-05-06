#include "frustum_sprite.h"
#include "gl/GL.h"
#include "ds/params/draw_params.h"
#include "cinder/CinderMath.h"

namespace ds {
	namespace ui {

	FrustumSprite::FrustumSprite( SpriteEngine& engine, float fov, const cinder::Vec3f &center, float _near, float _far, float w, float h )
		: Sprite(engine, w, h)
	{
		if ( w == 0 )
			w = engine.getWorldWidth();
		if ( h == 0 )
			h = engine.getWorldHeight();

		float cam_z = h / (2.0f * cinder::math<float>::sin( fov*(float)M_PI/180.0f / 2.0f ) );

		float near_h = h * _near / cam_z;
		float near_w = w * _near / cam_z;

		float L = -w/2 - center.x;
		float R =  w/2 - center.x;
		float B = -h/2 + center.y;
		float T =  h/2 + center.y;

		mL = L * _near / cam_z;
		mR = R * _near / cam_z;
		mB = B * _near / cam_z;
		mT = T * _near / cam_z;
		mN = _near;
		mF = _far;

		mCamera.lookAt( cinder::Vec3f( 0, 0, cam_z ), cinder::Vec3f( 0, 0, 0 ) );
		//assert( r - l == near_w );
		//assert( t - b == near_h );
	}

	void FrustumSprite::drawClient( const ci::Matrix44f &trans, const DrawParams &drawParams ) {
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();

			glLoadIdentity();
			glFrustum( mL, mR, mB, mT, mN, mF );

			ci::Matrix44f new_trans = mCamera.getModelViewMatrix();
			ci::Matrix44f totalTransformation = new_trans*mTransformation;

			if (getTransparent() == false) {
				ci::gl::color(mColor.r, mColor.g, mColor.b, mOpacity*drawParams.mParentOpacity);
				ci::gl::pushModelView();

					glLoadIdentity();
					ci::gl::multModelView(totalTransformation);
					ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight));

				ci::gl::popModelView();
			}

			DrawParams dParams = drawParams;
			dParams.mParentOpacity *= mOpacity;
			for ( auto it = mChildren.begin(), it2 = mChildren.end(); it != it2; ++it ) {
				(*it)->drawClient(totalTransformation, dParams);
			}

		glMatrixMode( GL_PROJECTION );
		glPopMatrix();

		glMatrixMode( GL_MODELVIEW );
	}

	} // namespace ui
} //namespace ds


/* vim: set noet fenc= ff=dos sts=0 sw=4 ts=4 : */
