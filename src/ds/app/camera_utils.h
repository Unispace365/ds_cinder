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
		CameraPick(const ci::Ray& pickRay, const ci::vec3& cameraDirection)
			: mPickRay(pickRay)
			, mCameraDirection(cameraDirection)
		{}

		//const ci::vec3&			getScreenPoint() const;
		//ci::Camera&				getCamera(){ return mCamera; }
		const ci::Ray&			getPickRay() const { return mPickRay; }
		const ci::vec3&			getCameraDirection() const { return mCameraDirection; }

	private:
		const ci::Ray&				mPickRay;
		const ci::vec3&				mCameraDirection;
		//ci::Camera&				mCamera;
		//const ci::vec3			mScreenPoint;
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
