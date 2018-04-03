#include "stdafx.h"

#include "perspective_layout.h"


namespace ds {
namespace ui {

PerspectiveLayout::PerspectiveLayout(ds::ui::SpriteEngine& e, const float fov, const bool autoClip, const float autoClipDepthRange)
	: ds::ui::LayoutSprite(e)
	, mPerspEnabled(true)
	, mFov(fov)
	, mAutoClip(autoClip)
	, mAutoClipDepthRange(autoClipDepthRange)
	, mNearClip(-1000.0f)
	, mFarClip(1000.0f)
{
}

ds::ui::Sprite* PerspectiveLayout::getHit(const ci::vec3& point) {
	if(mPerspEnabled && contains(point)) {
		ds::CameraPick cp = ds::CameraPick(mEngine, mViewport, mCam, point);
		return getPerspectiveHit(cp);
	} else {
		return ds::ui::Sprite::getHit(point);
	}
}

void PerspectiveLayout::updateCam(const ci::mat4& transform) {
	auto screenSize = glm::vec2(ci::app::getWindowSize());
	auto scaleW = screenSize.x / mEngine.getSrcRect().getWidth();
	auto scaleH = screenSize.y / mEngine.getSrcRect().getHeight();

	mCam = ci::CameraPersp((int)getWidth(), (int)getHeight(), mFov, mNearClip, mFarClip);
	if(mAutoClip) {
		float nearClip = mCam.getEyePoint().z - mAutoClipDepthRange / 2.0f;
		if(nearClip < 0.1f) nearClip = 0.1f;
		mCam.setNearClip(nearClip);
		mCam.setFarClip(mCam.getEyePoint().z + mAutoClipDepthRange / 2.0f);
	}
	mCam.setWorldUp(ci::vec3(0.0f, 1.0f, 0.0f));

	// Needs to be in screeen space,
	ci::vec2 vpSize(getWidth(), getHeight());
	ci::vec2 vpPos(getPosition().x, getPosition().y);
	mViewport = ci::Rectf(vpPos, vpPos + vpSize);

	auto vp = mViewport.transformed(ci::mat3(transform));
	vp.offset(ci::vec2(getGlobalPosition()));

	auto lowerLeft = vp.getLowerLeft() * ci::vec2(scaleW, scaleH);
	lowerLeft.y = screenSize.y - lowerLeft.y;
	mViewport = ci::Rectf(lowerLeft, lowerLeft + vp.getSize() * ci::vec2(scaleW, scaleH));
}

void PerspectiveLayout::drawClient(const ci::mat4& transformMatrix, const ds::DrawParams& drawParams) {
	if(!mPerspEnabled) {
		ds::ui::LayoutSprite::drawClient(transformMatrix, drawParams);
		return;
	}

	updateCam(transformMatrix);

	ci::gl::ScopedMatrices scm;
	ci::gl::ScopedViewport svp(mViewport.getUpperLeft(), mViewport.getSize());
	ci::gl::setMatrices(mCam);

	auto trans = ci::mat4();
	trans = glm::scale(trans, ci::vec3(1.0f, -1.0f, 1.0f));
	trans = glm::translate(trans, ci::vec3(0.0f, -getHeight(), 0.0f));

	ds::ui::LayoutSprite::drawClient(trans, drawParams);
}

}  // namespace ui
}  // namespace ds
