#pragma once
#include <ds/ui/sprite/sprite.h>

namespace waffles {

/**
 * \class waffles::ViewerCreationArgs
 */
class ViewerCreationArgs {
  public:
	enum { kViewLayerNormal = 0, kViewLayerBackground, kViewLayerTop };

	ViewerCreationArgs()
	  : mLocation(-1.0f, -1.0f, 0.0f)
	  , mSize(-1.0f, -1.0f, 0.0f)
	  , mViewLayer(kViewLayerNormal)
	  , mStartWidth(0.0f)
	  , mEnforceMinSize(true)
	  , mFromCenter(true)
	  , mCanFullscreen(true)
	  , mIsFullscreen(false)
	  , mCanDetach(false)
	  , mIsDetached(false)
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
	  , mCanFullscreen(true)
	  , mIsFullscreen(fullscreen)
	  , mCanDetach(false)
	  , mIsDetached(false)
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

	ds::model::ContentModelRef mMediaRef;					//
	ds::ui::Sprite*			   mTargetSprite = nullptr;		//
	std::string				   mViewType;					//
	ci::vec3				   mLocation;					//
	ci::vec3				   mSize;						//
	int						   mViewLayer;					//
	float					   mStartWidth;					//
	bool					   mEnforceMinSize;				//
	bool					   mFromCenter;					//
	bool					   mCanFullscreen;				// Whether or not the viewer can go fullscreen.
	bool					   mIsFullscreen;				// Whether or not the viewer should be fullscreen.
	bool					   mCanDetach;					// Whether or not the viewer can be detached.
	bool					   mCanAttach;					// Whether or not the viewer can be attached.
	bool					   mIsDetached;					// Whether or not the viewer is detached.
	bool					   mShowFullscreenController;	//
	bool					   mShowPresentationController; //
	bool					   mCheckBounds;				//
	bool					   mUseHotspots;				//
	bool					   mAmSlideContent = false;		//

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
	inline static ViewerCreationArgs
	detached(ds::model::ContentModelRef newMedia, const std::string viewType,
								   ci::vec3 location = ci::vec3(-1.0f, -1.0f, 0.0f), int viewLayer = kViewLayerNormal,
								   float startWidth = 0.0f, const bool fromCenter = true, const bool fullscreen = false,
								   const bool checkBounds = true) {
		auto result = ViewerCreationArgs(newMedia, viewType, location, viewLayer, startWidth, fromCenter, fullscreen,
										 checkBounds);
		result.mCanDetach = true;
		result.mCanAttach  = false;
		result.mIsDetached = true;
		return result;
	}
};

} // namespace waffles
