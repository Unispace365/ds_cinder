#pragma once
#ifndef DS_UI_LAYOUT_PERSPECTIVE_LAYOUT
#define DS_UI_LAYOUT_PERSPECTIVE_LAYOUT


#include <ds/ui/layout/layout_sprite.h>


namespace ds {
namespace ui {

/**
 * \class PerspectiveLayout
 *			Renders children in 3d.
 *	Caveats:
 *		- Centers the perspective view on the center of this sprite
 *		- Creates a viewport the size of this sprite (effectively clips)
 *		- Nesting PerspectiveLayouts may completely break
 *		- Clipping planes in the parent tree may have undesired effects
 */
class PerspectiveLayout : public ds::ui::LayoutSprite {
  public:
	PerspectiveLayout(ds::ui::SpriteEngine& e, const float fov = 30.0f, const bool autoClip = true,
					  const float autoClipDepthRange = 3000.0f);

	/// Get the Perspective camera. Note that this is recreated each frame based on sprite size, fov and clip ranges
	ci::CameraPersp getCamera() { return mCam; }

	/// The field of view value in degrees for the camera
	const float getFov() { return mFov; }
	void		setFov(const float newFieldOfView) { mFov = newFieldOfView; }

	/// If the clip near/far is auto-centered on the eye distance
	const bool getIsAutoClip() { return mAutoClip; }

	/// The clip near/far is auto-centered on the eye distance. The amount is set with the range value
	void setAutoClip(const bool doAutoClip) { mAutoClip = doAutoClip; }

	/// The depth range around the eye point for near/far clip if auto clip is on
	const float getAutoClipDepthRange() { return mAutoClipDepthRange; }
	void		setAutoClipDepthRange(const float clipRange) { mAutoClipDepthRange = clipRange; }

	/// If not in auto-clip mode, returns the value for the near clip distance
	const float getNearClip() { return mNearClip; }
	void		setNearClip(const float nearClip) { mNearClip = nearClip; }

	/// If not in auto-clip mode, returns the value for the far clip distance
	const float getFarClip() { return mFarClip; }
	void		setFarClip(const float farClip) { mFarClip = farClip; }

	/// Enable or disable perspective mode
	void	   setPerspectiveEnabled(const bool doPerspective) { mPerspEnabled = doPerspective; }
	const bool getPerspectiveEnabled() { return mPerspEnabled; }

  protected:
	virtual void			drawClient(const ci::mat4& transformMatrix, const ds::DrawParams& drawParams) override;
	virtual ds::ui::Sprite* getHit(const ci::vec3& point) override;

	void updateCam(const ci::mat4& transform);

  private:
	ci::CameraPersp mCam;
	ci::ivec2		mVpPos;
	ci::ivec2		mVpSize;
	ci::Rectf		mViewport;

	float mFov, mNearClip, mFarClip;

	bool  mAutoClip;
	float mAutoClipDepthRange;

	float mWorldHeight;
	float mDstHeight;
	float mDstWidth;
	float mSrcHeight;
	float mSrcWidth;
	float mScaleW;
	float mScaleH;

	bool mPerspEnabled;
};

}  // namespace ui
}  // namespace ds

#endif
