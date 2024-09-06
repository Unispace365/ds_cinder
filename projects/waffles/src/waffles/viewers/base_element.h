#pragma once

#include <functional>

#include <cinder/Rect.h>

#include <ds/content/content_model.h>
#include <ds/ui/panel/base_panel.h>

#include "waffles/model/viewer_creation_args.h"

namespace waffles {

class TitledMediaViewer;

/**
 * \class waffles::BaseElement
 *			A base class for anything that appears onscreen.
 */
class BaseElement : public ds::ui::BasePanel {
  public:
	BaseElement(ds::ui::SpriteEngine& g);

	/// Set the content for this panel
	virtual void setMedia(const ds::model::ContentModelRef& newMedia) final;
	/// Not all viewer types use this, so use with discretion
	ds::model::ContentModelRef getMedia() { return mMediaRef; }

	/// If true, will participate in "arrange" commands from the rest of the app. If false, will close when a arrange
	/// event is requested
	bool canArrange();

	/// If true, can be resized
	bool canResize();

	/// If this element can go into fullscreen mode
	bool canFullScreen();

	/// This should only be set by ViewerContoller, which manages fullscreen-ness
	void setIsFullscreen(const bool isFullscreen);
	bool getIsFullscreen();

	/// How many of this specific type of viewers can be onscreen at a time
	const int getMaxNumberOfThisType();

	const std::string& getViewerType();

	/// If this element encountered an error it can't recover from. If true, this element may be removed at the next
	/// layout request or other event
	bool getIsFatalErrored();

	/// For viewerController to clean up this element
	virtual void setCloseRequestCallback(std::function<void(void)> func) final;

	/// For viewerContoller to manage the viewer list
	virtual void setActivatedCallback(std::function<void(void)> func) final;

	virtual void animateOn() final;
	virtual void animateOn(const float delay) final;

	/// Sets with layer this is on. See ViewerCreationArgs for possible values
	virtual void setViewerLayer(const int viewerLayer) final;
	const int	 getViewerLayer();

	/// Sets the size/position of this viewer when it's not in fullscreen mode (so it can return to it after being
	/// fullscreened)
	void	  setUnfullscreenRect(ci::Rectf recty);
	ci::Rectf getUnfullscreenRect();

	virtual int getMediaRotation() { return 0; }

	void setCreationArgs(ViewerCreationArgs args);

	virtual void playContent() {}
	virtual void pauseContent() {}
	virtual void toggleMute() {}
	virtual void mute() {}
	virtual void unmute() {}

	virtual void showTitle() {}
	virtual void hideTitle() {}
	virtual void toggleTitle() {}

	virtual void showInnerSideBar() {}
	virtual void hideInnerSideBar() {}
	virtual void toggleInnerSideBar() {}

  protected:
	// The layer has been changed (see ViewerCreationArgs for layers)
	virtual void onViewerLayerSet() {}

	// The creation args have been set (this is the place to set page, volume, looping, autoplay, etc)
	virtual void onCreationArgsSet() {}

	// Override to know when the media has been set
	virtual void onMediaSet() {}

	// Set fullscreen has been called. Note: The viewer may still be animating to the final position and size when this
	// is called
	virtual void onFullscreenSet() {}

	virtual void onPanelActivated() override;

	friend class ViewerController;

	bool					   mCanArrange;
	bool					   mCanResize;
	bool					   mCanFullscreen;
	bool					   mIsFullscreen;
	std::string				   mViewerType;
	int						   mMaxViewersOfThisType;
	int						   mViewerLayer;
	ci::Rectf				   mUnfullscreenRect;
	ds::model::ContentModelRef mMediaRef;
	ViewerCreationArgs		   mCreationArgs;

	bool mFatalError;

	std::function<void(void)> mCloseRequestCallback;
	std::function<void(void)> mActivatedCallback;
};

} // namespace waffles
