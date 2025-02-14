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
	BaseElement(ds::ui::SpriteEngine& g, const std::string& eventChannel = "");

	/// Set the content for this panel
	virtual void setMedia(const ds::model::ContentModelRef& newMedia) final;
	
	/// Not all viewer types use this, so use with discretion
	ds::model::ContentModelRef getMedia() const { return mMediaRef; }

	/// Get the size of the media, if available. For viewers without media, the size of its layout will be returned instead.
	ci::vec2 getMediaSize() const;

	/// If true, will participate in "arrange" commands from the rest of the app. If false, will close when a arrange
	/// event is requested
	bool canArrange() const;

	void allowArrange(bool allow) { mCanArrange = allow; }

	/// If true, can be resized
	bool canResize() const;

	void allowResize(bool allow) { mCanResize = allow; }

	/// If this element can go into fullscreen mode
	bool canFullScreen() const;

	void allowFullscreen(bool allow);

	/// This should only be set by ViewerContoller, which manages fullscreen-ness
	void setIsFullscreen(bool isFullscreen);
	bool getIsFullscreen() const;

	/// If this element can be detached from the layout
	bool canDetach() const;

	void allowDetach(bool allow);

	/// If this element can be attached to the layout
	bool canAttach() const;

	void allowAttach(bool allow);

	/// If this element is detached from the layout
	void setIsDetached(bool isDetached);
	bool getIsDetached() const;

	/// How many of this specific type of viewers can be onscreen at a time
	int getMaxNumberOfThisType() const;

	const std::string& getViewerType() const;

	/// If this element encountered an error it can't recover from. If true, this element may be removed at the next
	/// layout request or other event
	bool getIsFatalErrorred() const;

	/// For viewerController to clean up this element
	virtual void setCloseRequestCallback(std::function<void(void)> func) final;

	virtual void close();

	/// For viewerController to manage the viewer list
	virtual void setActivatedCallback(std::function<void(void)> func) final;

	virtual void animateOn() final;
	virtual void animateOn(float delay) final;

	/// Sets with layer this is on. See ViewerCreationArgs for possible values
	virtual void setViewerLayer(int viewerLayer) final;
	int          getViewerLayer() const;

	/// Sets the size/position of this viewer when it's not in fullscreen mode (so it can return to it after being
	/// fullscreened)
	void	  setUnfullscreenRect(ci::Rectf recty);
	ci::Rectf getUnfullscreenRect() const;

	virtual void setToFullscreen(bool immediate, bool showController);

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

	bool setAvailableSize(const ci::vec2& size, float& minWidth, float& minHeight, float& maxWidth, float& maxHeight,
						  bool favorWidthOverHeight) override;

	void fitInsideArea(const ci::Rectf& area) override;

	struct Padding {
		float left{0};
		float right{0};
		float top{0};
		float bottom{0};
	};


	using PaddingFlag = int;

	static constexpr PaddingFlag PaddingNone   = 0;
	static constexpr PaddingFlag PaddingLeft   = 1 << 0;
	static constexpr PaddingFlag PaddingRight  = 1 << 1;
	static constexpr PaddingFlag PaddingTop	   = 1 << 2;
	static constexpr PaddingFlag PaddingBottom = 1 << 3;
	static constexpr PaddingFlag PaddingAll	   = PaddingLeft | PaddingRight | PaddingTop | PaddingBottom;


	virtual void setDetachedPaddingFlags(const PaddingFlag& flags);
	virtual void setAttachedPaddingFlags(const PaddingFlag& flags);
	virtual PaddingFlag getDetachedPaddingFlags();
	virtual PaddingFlag getAttachedPaddingFlags();

	virtual ci::vec2 getBorderPadding();
	virtual ci::vec2 getBorderOffset();
	virtual Padding	 getActivePadding(bool reverse = false);
	virtual Padding	 getAttachedPadding();
	virtual Padding	 getDetachedPadding();
	

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

	// Detached has been set.
	virtual void onDetachedSet() {}

	void onPanelActivated() override;
	void onParentSet() override;

	friend class ViewerController;

	bool                       mCanArrange;
	bool                       mCanResize;
	bool                       mCanFullscreen;
	bool                       mIsFullscreen;
	bool                       mCanDetach;
	bool                       mCanAttach{};
	bool                       mIsDetached;
	std::string                mViewerType;
	int                        mMaxViewersOfThisType;
	int                        mViewerLayer{};
	ci::Rectf                  mUnfullscreenRect;
	ds::model::ContentModelRef mMediaRef;
	ViewerCreationArgs         mCreationArgs;
	ds::EventClient            mEventClient;
	PaddingFlag                mDetachedPadding = PaddingAll;
	PaddingFlag                mAttachedPadding = PaddingNone;

	bool mFatalError;

	std::function<void(void)> mCloseRequestCallback;
	std::function<void(void)> mActivatedCallback;
};

} // namespace waffles
