#include <ds/app/camera_utils.h>

#include <cinder/gl/gl.h>

namespace ds {

/**
 * \class ds::CameraPick
 */
CameraPick::CameraPick(	ci::Camera& c, const ci::vec3& screenPt,
												const float screenWidth, const float screenHeight)
	: mCamera(c)
	, mScreenPt(screenPt)
	, mScreenW(screenWidth)
	, mScreenH(screenHeight)
{
}

const ci::vec3& CameraPick::getScreenPt() const
{
	return mScreenPt;
}

ci::vec2 CameraPick::worldToScreen(const ci::vec3 &worldCoord) const
{
	return mCamera.worldToScreen(worldCoord, mScreenW, mScreenH);
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
	invMVP = glm::inverse(invMVP);//.invert();

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
