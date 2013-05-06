#include <ds/app/camera_utils.h>

#include <cinder/gl/gl.h>

namespace ds {

/**
 * \class ds::CameraPick
 */
CameraPick::CameraPick(	ci::Camera& c, const ci::Vec3f& screenPt,
												const float screenWidth, const float screenHeight)
	: mCamera(c)
	, mScreenPt(screenPt)
	, mScreenW(screenWidth)
	, mScreenH(screenHeight)
{
}

const ci::Vec3f& CameraPick::getScreenPt() const
{
	return mScreenPt;
}

ci::Vec2f CameraPick::worldToScreen(const ci::Vec3f &worldCoord) const
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
	mProjection = ci::gl::getProjection();
	mViewport = ci::gl::getViewport();
}

ci::Vec3f ScreenToWorld::translate(const ci::Vec3f & point)
{
	// Find near and far plane intersections
	ci::Vec3f point3f = ci::Vec3f((float)point.x, mWindowSize.getHeight() * 0.5f - (float)point.y, 0.0f);
	ci::Vec3f nearPlane = unproject(point3f);
	ci::Vec3f farPlane = unproject(ci::Vec3f(point3f.x, point3f.y, 1.0f));

	// Calculate X, Y and return point
	float theta = (0.0f - nearPlane.z) / (farPlane.z - nearPlane.z);
	return ci::Vec3f(
		nearPlane.x + theta * (farPlane.x - nearPlane.x), 
		nearPlane.y + theta * (farPlane.y - nearPlane.y), 
		0.0f
	);
}

ci::Vec3f ScreenToWorld::unproject(const ci::Vec3f & point)
{
	// Find the inverse Modelview-Projection-Matrix
	ci::Matrix44f invMVP = mProjection * mModelView;
	invMVP.invert();

	// Transform to normalized coordinates in the range [-1, 1]
	ci::Vec4f				pointNormal;
	pointNormal.x = (point.x - mViewport.getX1()) / mViewport.getWidth() * 2.0f - 1.0f;
	pointNormal.y = (point.y - mViewport.getY1()) / mViewport.getHeight() * 2.0f;
	pointNormal.z = 2.0f * point.z - 1.0f;
	pointNormal.w = 1.0f;

	// Find the object's coordinates
	ci::Vec4f				pointCoord = invMVP * pointNormal;
	if (pointCoord.w != 0.0f)
		pointCoord.w = 1.0f / pointCoord.w;

	// Return coordinate
	return ci::Vec3f(
		pointCoord.x * pointCoord.w, 
		pointCoord.y * pointCoord.w, 
		pointCoord.z * pointCoord.w
	);
}

} // namespace ds
