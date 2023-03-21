#include "stdafx.h"

#include "perspective_layout.h"

namespace ds { namespace ui {

	PerspectiveLayout::PerspectiveLayout(ds::ui::SpriteEngine& e, const float fov, const bool autoClip,
										 const float autoClipDepthRange)
	  : ds::ui::LayoutSprite(e)
	  , mPerspEnabled(true)
	  , mDstWidth(e.getDstRect().getWidth())
	  , mDstHeight(e.getDstRect().getHeight())
	  , mSrcWidth(e.getSrcRect().getWidth())
	  , mSrcHeight(e.getSrcRect().getHeight())
	  , mScaleW(mDstWidth / mSrcWidth)
	  , mScaleH(mDstHeight / mSrcHeight)
	  , mFov(fov)
	  , mAutoClip(autoClip)
	  , mAutoClipDepthRange(autoClipDepthRange)
	  , mNearClip(-1000.0f)
	  , mFarClip(1000.0f) {}

	ds::ui::Sprite* PerspectiveLayout::getHit(const ci::vec3& point) {
		if (mPerspEnabled && contains(point)) {
			ds::CameraPick cp = ds::CameraPick(mEngine, mViewport, mCam, point);
			return getPerspectiveHit(cp);
		} else {
			return ds::ui::Sprite::getHit(point);
		}
	}

	void PerspectiveLayout::updateCam(const ci::mat4& transform) {
		auto screenSize = glm::vec2(ci::app::getWindowSize());
		mScaleW			= screenSize.x / mSrcWidth;
		mScaleH			= screenSize.y / mSrcHeight;

		mCam = ci::CameraPersp(static_cast<int>(getWidth()), static_cast<int>(getHeight()), mFov, mNearClip, mFarClip);
		if (mAutoClip) {
			mNearClip = mCam.getEyePoint().z - mAutoClipDepthRange / 2.0f;
			if (mNearClip < 0.1f) mNearClip = 0.1f;
			mFarClip = mCam.getEyePoint().z + mAutoClipDepthRange / 2.0f;

			mCam.setNearClip(mNearClip);
			mCam.setFarClip(mFarClip);
		}

		mCam.setWorldUp(ci::vec3(0.0f, 1.0f, 0.0f));

		// Needs to be in screeen space,
		auto gp	  = ci::vec2(getGlobalPosition());
		mViewport = ci::Rectf(gp, gp + ci::vec2(getSize() * getScale()));

		mVpSize	 = ci::vec2(getWidth(), getHeight()) * ci::vec2(mScaleW, mScaleH);
		mVpPos	 = ci::vec2(getGlobalPosition()) * ci::vec2(mScaleW, mScaleH);
		mVpPos.y = static_cast<int>(screenSize.y - (mVpPos.y + mVpSize.y));
	}

	void PerspectiveLayout::drawClient(const ci::mat4& transformMatrix, const ds::DrawParams& drawParams) {
		if (!mPerspEnabled) {
			ds::ui::LayoutSprite::drawClient(transformMatrix, drawParams);
			return;
		}

		updateCam(transformMatrix);

		ci::gl::ScopedMatrices scm;
		ci::gl::ScopedViewport svp(mVpPos, mVpSize);
		ci::gl::setMatrices(mCam);

		auto trans = ci::mat4();
		trans	   = glm::scale(trans, ci::vec3(1.0f, -1.0f, 1.0f));
		trans	   = glm::translate(trans, ci::vec3(0.0f, -getHeight(), 0.0f));

		ds::ui::LayoutSprite::drawClient(trans, drawParams);
	}

}} // namespace ds::ui
