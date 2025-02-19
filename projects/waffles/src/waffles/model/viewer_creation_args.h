#pragma once
#include <ds/ui/sprite/sprite.h>

#include <utility>

namespace waffles {

/**
 * \class waffles::ViewerCreationArgs
 */
class ViewerCreationArgs {
  public:
	enum { kViewLayerNormal = 0, kViewLayerBackground, kViewLayerTop };

	ViewerCreationArgs() = default;

	ViewerCreationArgs(ds::model::ContentModelRef newMedia, const std::string& viewType,
					   const ci::vec3& location = ci::vec3(-1.0f, -1.0f, 0.0f), int viewLayer = kViewLayerNormal,
					   float startWidth = 0.0f, const bool fromCenter = true, const bool fullscreen = false,
					   const bool checkBounds = true)
	  : mMediaRef(std::move(newMedia))
	  , mViewType(viewType)
	  , mLocation(location)
	  , mViewLayer(viewLayer)
	  , mStartWidth(startWidth)
	  , mFromCenter(fromCenter)
	  , mIsFullscreen(fullscreen)
	  , mCheckBounds(checkBounds) {}

	ds::model::ContentModelRef mMediaRef;					 //
	ds::ui::Sprite*			   mTargetSprite = nullptr;		 //
	std::string				   mViewType;					 //
	ci::vec3				   mLocation{-1, -1, 0};		 //
	ci::vec3				   mSize{-1, -1, 0};			 //
	int						   mViewLayer{kViewLayerNormal}; //
	int						   mVolume{50};					 // 0-100
	int						   mPage{1};					 // for PDF pages
	float					   mStartWidth{0};				 //
	bool					   mEnforceMinSize{true};		 // Defaults to true.
	bool					   mFromCenter{true};			 // Defaults to true.
	bool					   mCanFullscreen{true};		 // Whether or not the viewer can go fullscreen.
	bool					   mIsFullscreen{false};		 // Whether or not the viewer should be fullscreen.
	bool					   mCanDetach{true};  // Whether or not the viewer can be detached. Defaults to true.
	bool					   mCanAttach{false}; // Whether or not the viewer can be attached. Defaults to false.
	bool					   mIsDetached{true}; // Whether or not the viewer is detached. Defaults to true.
	bool					   mShowFullscreenController{true};	  // Defaults to true.
	bool					   mShowPresentationController{true}; // Defaults to true.
	bool					   mCheckBounds{true};				  // Defaults to true.
	bool					   mUseHotspots{false};				  // Defaults to false.
	bool					   mAmSlideContent{false};			  // Defaults to false.
	bool					   mAutoStart{true};				  // Defaults to true.
	bool					   mLooped{true};					  // Defaults to true.
	bool					   mCloseOnVideoComplete{false};	  // Defaults to false.
	bool					   mMuted{false};					  // Defaults to false.
	double					   mVideoTimePosition{0};			  //
	bool					   mTouchEvents{true};				  // Defaults to true.
	bool					   mStartLocked{false};				  // Defaults to false.
	bool					   mStartDrawing{false};			  // Defaults to false.


	static ViewerCreationArgs detached(ds::model::ContentModelRef newMedia, const std::string& viewType,
									   const ci::vec3& location = ci::vec3(-1.0f, -1.0f, 0.0f),
									   int viewLayer = kViewLayerNormal, float startWidth = 0.0f,
									   const bool fromCenter = true, const bool fullscreen = false,
									   const bool checkBounds = true) {
		auto result = ViewerCreationArgs(std::move(newMedia), viewType, location, viewLayer, startWidth, fromCenter,
										 fullscreen, checkBounds);
		result.mCanDetach  = true;
		result.mCanAttach  = false;
		result.mIsDetached = true;
		return result;
	}
};

} // namespace waffles
