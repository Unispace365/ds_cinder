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
namespace ui {
class SpriteEngine;
class Sprite;
} // namespace ui

/**
 * \class ds::CameraPick
 * Utility for picking a sprite.
 */
class CameraPick {
public:
	CameraPick( const ds::ui::SpriteEngine& engine, const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint )
		: mPickRay( calculatePickRay(engine, cameraPersp, worldTouchPoint) )
		, mCameraDirection( glm::normalize( cameraPersp.getViewDirection()) )
	{}
	// Takes an additional viewport argument, to enable 3d picking for arbitrary 3d viewports
	CameraPick( const ds::ui::SpriteEngine& engine, const ci::Rectf& viewport , const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint )
		: mPickRay( calculatePickRay(engine, viewport, cameraPersp, worldTouchPoint) )
		, mCameraDirection( glm::normalize( cameraPersp.getViewDirection()) )
	{}

	const bool					testHitSprite( ds::ui::Sprite* sprite, ci::vec3& hitWorldPos ) const;
	const float					calcHitDepth( const ci::vec3& hitWorldPos ) const;

	static const ci::Ray		calculatePickRay( const ds::ui::SpriteEngine& engine, const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint );
	static const ci::Ray		calculatePickRay( const ds::ui::SpriteEngine& engine, const ci::Rectf& viewport, const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint );

protected:
	const ci::Ray				mPickRay;
	const ci::vec3				mCameraDirection;
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
