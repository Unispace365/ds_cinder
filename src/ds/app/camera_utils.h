#pragma once
#ifndef DS_APP_CAMERAUTILS_H
#define DS_APP_CAMERAUTILS_H

#include <cinder/Camera.h>
#include <cinder/Rect.h>
#include <cinder/Vector.h>
#include <cinder/Matrix.h>
#include <cinder/Matrix33.h>
#include <cinder/Matrix44.h>

namespace ds {

/**
 * \class ds::CameraPick
 * Utility for picking a sprite.
 */
class CameraPick
{
	public:
		CameraPick(	ci::Camera&, const ci::vec3& screenPt,
								const float screenWidth, const float screenHeight);

		const ci::vec3&		getScreenPt() const;
		ci::vec2				worldToScreen(const ci::vec3 &worldCoord) const;

  private:
		ci::Camera&				mCamera;
		const ci::vec3			mScreenPt;
		const float				mScreenW,
								mScreenH;
};

/**
 * \class ds::ScreenToWorld
 * Utility for converting screen points to world points.
 * Pulled from cinder forums:
 *	http://forum.libcinder.org/topic/converting-the-mouse-position-to-3d-world-cordinates
 * NOTE: Can't get it to work. So, it'd be nice, but...
 */
class ScreenToWorld
{
	public:
		ScreenToWorld();

		void					setScreenSize(const float width, const float height);
		void					update();

		ci::vec3				translate(const ci::vec3&);

  private:
		ci::vec3				unproject(const ci::vec3&);

		ci::mat4			mModelView;
		ci::mat4			mProjection;
		ci::Area				mViewport;
		ci::Rectf				mWindowSize;
};

} // namespace ds

#endif // DS_APP_CAMERAUTILS_H
