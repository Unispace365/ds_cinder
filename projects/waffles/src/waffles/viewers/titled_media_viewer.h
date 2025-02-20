#pragma once

#include "waffles/viewers/drawing/drawing_area.h"

#include "waffles/viewers/base_element.h"
#include <ds/ui/button/image_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/media/media_player.h>

namespace waffles {
class PresetMediaHotspotRef;


/**
 * \class waffles::TitledMediaViewer
 *			A single media viewer with a title and a close button
 */
class TitledMediaViewer : public BaseElement {
  public:
	enum class BoundsMode { kSpriteEdge, kMediaEdge };
	TitledMediaViewer(ds::ui::SpriteEngine& g, const std::string& eventChannel = "",
					  const std::string& layoutPath = "waffles/viewer/titled_media_viewer.xml");

	void onMediaSet() override;

	void showTitle() override;
	void hideTitle() override;
	void toggleTitle() override;

	void showInnerSideBar() override;
	void hideInnerSideBar() override;
	void toggleInnerSideBar() override;

	void setToFullscreen(const bool immediate, const bool showController) override;
	void checkBounds(const bool animate) override;

	// 0 = normal, 1 = 90 degrees, 2 = 180 degrees, 3 = 270 degrees
	int getMediaRotation() override { return mMediaRotation; }

	// May be nullptr, use caution
	ds::ui::MediaPlayer* getMediaPlayer() const { return mMediaPlayer; }

	bool		 getIsDrawingMode() const { return mDrawingMode; }
	void		 toggleDrawing();
	DrawingArea* getDrawingArea() const { return mDrawingArea; }

	/// In case viewer gets destroyed while drawing was happening
	void cleanupDrawing(bool clearDrawArea);
	/// Will only affect content that can be played (videos)
	void playContent() override;
	/// If this was a video and started as just a thumbnail with a play icon, start the actual video
	void startVideo() const;
	void pauseContent() override;
	void toggleMute() override;
	void mute() override;
	void unmute() override;

	ViewerCreationArgs getDuplicateCreationArgs() const;

	void setInterfaceLocked(bool isLocked) const;
	bool isInterfaceLocked() const {
		if (mMediaPlayer) return mMediaPlayer->isInterfaceLocked();
		return false;
	}

	// rotates media 90 degrees clockwise
	void rotateMedia();

  protected:
	void processAllowedButtons() const;
	void processAllowedTouch();
	void userInputReceived() override;
	void onLayout() override;
	void onCreationArgsSet() override;
	void onFullscreenSet() override;
	void onDetachedSet() override;


	void		loadHotspots();
	void		layoutHotspots() const;
	void		calculateSizeLimits();
	void		toggleOptions();
	static void setKeyboardButtonImage(const std::string& imagePath, ds::ui::ImageButton* keyboardBtn);

	ds::ui::SmartLayout*			  mRootLayout  = nullptr;
	ds::ui::MediaPlayer*			  mMediaPlayer = nullptr;
	DrawingArea*					  mDrawingArea = nullptr;
	std::vector<ds::ui::SmartLayout*> mHotspots;

	bool	   mDrawingMode			 = false;
	bool	   mShowingOptions		 = false;
	bool	   mShowingInnerSideBar	 = false;
	bool	   mShowingTitle		 = false;
	bool	   mShowingKeyboard		 = false;
	bool	   mInitialLoadError	 = false;
	bool	   mShowingVideo		 = false;
	bool	   mShowingWeb			 = false;
	bool	   mShowingWebCam		 = false;
	BoundsMode mFullscreenBoundsMode = BoundsMode::kMediaEdge;
	BoundsMode mNormalBoundsMode	 = BoundsMode::kSpriteEdge;
	// std::string mMediaPropertyKey = "media";

	// 0 = normal, 1 = 90 degs, 2 = 180 degs, 3 = 270
	int mMediaRotation = 0;

	ds::time::Callback mPlayerLoadedTimer;
	ds::time::Callback mControlsTimeoutTimer;
};

} // namespace waffles
