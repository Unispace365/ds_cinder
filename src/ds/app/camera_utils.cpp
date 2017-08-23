#include "stdafx.h"

#include "ds/app/camera_utils.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"

#include <cinder/gl/gl.h>

namespace ds {

const ci::Ray CameraPick::calculatePickRay( const ds::ui::SpriteEngine& engine, const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint ) {
	// Note: Again, we are using the actual window size here, not
	// the Engine mDstRect size.
	auto screenSize = glm::vec2( ci::app::getWindowSize() );

	// Sigh... convert world coordinate back to screen coordinate
	const ci::vec2 srcOffset = engine.getSrcRect().getUpperLeft();
	const ci::vec2 screenScale = screenSize / engine.getSrcRect().getSize();
	const ci::vec2 screenPoint = (ci::vec2(worldTouchPoint) - srcOffset) * screenScale;

	// We need to flip the y screen coordinate because OpenGL
	// Defines 0, 0 to be the lower left corner of the viewport
	const auto viewportPoint = glm::vec3(screenPoint.x, screenSize.y-screenPoint.y, 0.0f);

	// Compute the pick Ray.  We need to unproject the 
	// viewport point into camera-space coordinates
	const auto viewMat = cameraPersp.getViewMatrix();
	const auto projMat = cameraPersp.getProjectionMatrix();
	const auto viewport = glm::vec4(0.0f, 0.0f, screenSize);
	glm::vec3 worldPosNear = glm::unProject(viewportPoint, viewMat, projMat, viewport);
	glm::vec3 rayDirection = glm::normalize(worldPosNear - cameraPersp.getEyePoint());

	return  ci::Ray(cameraPersp.getEyePoint(), rayDirection);
}

const ci::Ray CameraPick::calculatePickRay( const ds::ui::SpriteEngine& engine, const ci::Rectf& viewport, const ci::CameraPersp& cameraPersp, const ci::vec3& worldTouchPoint ) {
	// Alternate pick ray, calculated in using a world-space viewport.
	const auto rayPt = ci::vec2(worldTouchPoint.x - viewport.getX1(), viewport.getY2() - worldTouchPoint.y);
	auto ray = cameraPersp.generateRay(rayPt, viewport.getSize());
	ray.setOrigin(ray.getOrigin() + ci::vec3(viewport.getUpperLeft(), 0.0f));
	return ray;
}

const bool CameraPick::testHitSprite( ds::ui::Sprite* sprite, ci::vec3& hitWorldPos ) const {
	if (!sprite->isEnabled())
		return false;

	const float	w = sprite->getScaleWidth();
	const float h = sprite->getScaleHeight();

	if (w <= 0.0f || h <= 0.0f)
		return false;

	auto cornerA = sprite->localToGlobal(glm::vec3(0.0f, 0.0f, 0.0f));
	auto cornerB = sprite->localToGlobal(glm::vec3(sprite->getWidth(), 0.0f, 0.0f));
	auto cornerC = sprite->localToGlobal(glm::vec3(0.0f, sprite->getHeight(), 0.0f));

	auto v1 = cornerB - cornerA;
	auto v2 = cornerC - cornerA;

	auto norm = glm::normalize(glm::cross(v2, v1));

	float rayDist;
	bool intersectsPlane = mPickRay.calcPlaneIntersection(cornerA, norm, &rayDist);
	if (!intersectsPlane)
		return false;

	auto intersectPoint = mPickRay.calcPosition(rayDist);

	auto v = intersectPoint - cornerA;

	float dot1 = glm::dot(v, v1);
	float dot2 = glm::dot(v, v2);

	if (dot1 >= 0
		&& dot2 >= 0
		&& dot1 <= dot(v1, v1)
		&& dot2 <= dot(v2, v2)
		) {

		hitWorldPos = intersectPoint;
		return true;
	}

	return false;
}

const float CameraPick::calcHitDepth( const ci::vec3& hitWorldPos ) const {
	auto intersectVector = hitWorldPos - mPickRay.getOrigin();
	const float hitZ = glm::dot(intersectVector, mCameraDirection );
	return hitZ;
}


/**
 * \class ds::ScreenToWorld
 */
ScreenToWorld::ScreenToWorld()
{
}

void ScreenToWorld::setScreenSize(const float width, const float height)
{
	mWindowSize = ci::Rectf(0.0f, 0.0f, width, height);
}

void ScreenToWorld::update()
{
	mModelView = ci::gl::getModelView();
	mProjection = ci::gl::getProjectionMatrix();
	const auto& viewPort = ci::gl::getViewport();
	mViewport = ci::Area(viewPort.first, viewPort.second);
}

ci::vec3 ScreenToWorld::translate(const ci::vec3 & point)
{
	// Find near and far plane intersections
	ci::vec3 point3f = ci::vec3((float)point.x, mWindowSize.getHeight() * 0.5f - (float)point.y, 0.0f);
	ci::vec3 nearPlane = unproject(point3f);
	ci::vec3 farPlane = unproject(ci::vec3(point3f.x, point3f.y, 1.0f));

	// Calculate X, Y and return point
	float theta = (0.0f - nearPlane.z) / (farPlane.z - nearPlane.z);
	return ci::vec3(
		nearPlane.x + theta * (farPlane.x - nearPlane.x), 
		nearPlane.y + theta * (farPlane.y - nearPlane.y), 
		0.0f
	);
}

ci::vec3 ScreenToWorld::unproject(const ci::vec3 & point)
{
	// Find the inverse Modelview-Projection-Matrix
	ci::mat4 invMVP = mProjection * mModelView;
	invMVP = glm::inverse(invMVP);

	// Transform to normalized coordinates in the range [-1, 1]
	ci::vec4				pointNormal;
	pointNormal.x = (point.x - mViewport.getX1()) / mViewport.getWidth() * 2.0f - 1.0f;
	pointNormal.y = (point.y - mViewport.getY1()) / mViewport.getHeight() * 2.0f;
	pointNormal.z = 2.0f * point.z - 1.0f;
	pointNormal.w = 1.0f;

	// Find the object's coordinates
	ci::vec4				pointCoord = invMVP * pointNormal;
	if (pointCoord.w != 0.0f)
		pointCoord.w = 1.0f / pointCoord.w;

	// Return coordinate
	return ci::vec3(
		pointCoord.x * pointCoord.w, 
		pointCoord.y * pointCoord.w, 
		pointCoord.z * pointCoord.w
	);
}

} // namespace ds
