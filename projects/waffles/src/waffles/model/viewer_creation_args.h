#pragma once

namespace waffles {

/**
 * \class waffles::ViewerCreationArgs
 */
class ViewerCreationArgs {
  public:
	enum { kViewLayerNormal = 0, kViewLayerBackground, kViewLayerTop };

	ViewerCreationArgs()
		: mViewLayer(kViewLayerNormal)
		, mStartWidth(0.0f)
		, mEnforceMinSize(true)
		, mFromCenter(true)
		, mLocation(-1.0f, -1.0f, 0.0f)
		, mFullscreen(false)
		, mShowFullscreenController(true)
		, mShowPresentationController(true)
		, mCheckBounds(true)
		, mUseHotspots(false)
		, mVolume(50)
		, mPage(1)
		, mAutoStart(true)
		, mLooped(true)
		, mCloseOnVideoComplete(false)
		, mMuted(false)
		, mVideoTimePosition(0.0)
		, mTouchEvents(true)
		, mStartLocked(false)
		, mStartDrawing(false){};

	ViewerCreationArgs(ds::model::ContentModelRef newMedia, const std::string viewType,
					   ci::vec3 location = ci::vec3(-1.0f, -1.0f, 0.0f), int viewLayer = kViewLayerNormal,
					   float startWidth = 0.0f, const bool fromCenter = true, const bool fullscreen = false,
					   const bool checkBounds = true)
		: mMediaRef(newMedia)
		, mViewType(viewType)
		, mLocation(location)
		, mViewLayer(viewLayer)
		, mStartWidth(startWidth)
		, mEnforceMinSize(true)
		, mFromCenter(fromCenter)
		, mFullscreen(fullscreen)
		, mShowFullscreenController(true)
		, mShowPresentationController(true)
		, mCheckBounds(checkBounds)
		, mUseHotspots(false)
		, mVolume(50)
		, mPage(1)
		, mAutoStart(true)
		, mLooped(true)
		, mCloseOnVideoComplete(false)
		, mMuted(false)
		, mVideoTimePosition(0.0)
		, mTouchEvents(true)
		, mStartLocked(false)
		, mStartDrawing(false) {}

	ds::model::ContentModelRef mMediaRef;
	std::string				   mViewType;
	ci::vec3				   mLocation;
	int						   mViewLayer;
	float					   mStartWidth;
	bool					   mEnforceMinSize;
	bool					   mFromCenter;
	bool					   mFullscreen;
	bool					   mShowFullscreenController;
	bool					   mShowPresentationController;
	bool					   mCheckBounds;
	bool					   mUseHotspots;
	bool					   mAmSlideContent = false;

	// 0-100
	int mVolume;
	// for PDF pages
	int	   mPage;
	bool   mAutoStart;
	bool   mLooped;
	bool   mCloseOnVideoComplete;
	bool   mMuted;
	double mVideoTimePosition;
	bool   mTouchEvents;
	bool   mStartLocked;
	bool   mStartDrawing;
};

} // namespace waffles
