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
	TitledMediaViewer(ds::ui::SpriteEngine& g);

	virtual void onMediaSet() override;

	virtual void showTitle() override;
	virtual void hideTitle() override;
	virtual void toggleTitle() override;

	virtual void showInnerSideBar() override;
	virtual void hideInnerSideBar() override;
	virtual void toggleInnerSideBar() override;

	// 0 = normal, 1 = 90 degs, 2 = 180 degs, 3 = 270
	virtual int getMediaRotation() override { return mMediaRotation; }

	// May be nullptr, use caution
	ds::ui::MediaPlayer* getMediaPlayer() { return mMediaPlayer; }

	bool		 getIsDrawingMode() { return mDrawingMode; }
	void		 toggleDrawing();
	DrawingArea* getDrawingArea() { return mDrawingArea; }

	/// In case viewer gets destroyed while drawing was happening
	void cleanupDrawing(bool clearDrawArea);
	/// Will only affect content that can be played (videos)
	virtual void playContent() override;
	/// If this was a video and started as just a thumbnail with a play icon, start the actual video
	void		 startVideo();
	virtual void pauseContent() override;
	virtual void toggleMute() override;
	virtual void mute() override;
	virtual void unmute() override;

	ViewerCreationArgs getDuplicateCreationArgs();

	void setInterfaceLocked(bool isLocked);
	bool isInterfaceLocked() {
		if (mMediaPlayer) return mMediaPlayer->isInterfaceLocked();
		return false;
	}

	// rotates media 90 degrees clockwise
	void rotateMedia();

  private:
	virtual void userInputReceived() override;
	virtual void onLayout() override;
	virtual void onCreationArgsSet() override;
	virtual void onFullscreenSet() override;

	void loadHotspots();
	void layoutHotspots();
	void calculateSizeLimits();
	void toggleOptions();
	void setKeyboardButtonImage(std::string imageFile, ds::ui::ImageButton* keyboardBtn);

	ds::ui::SmartLayout*			  mRootLayout  = nullptr;
	ds::ui::MediaPlayer*			  mMediaPlayer = nullptr;
	DrawingArea*					  mDrawingArea = nullptr;
	std::vector<ds::ui::SmartLayout*> mHotspots;

	bool mDrawingMode		  = false;
	bool mShowingOptions	  = false;
	bool mShowingInnerSideBar = false;
	bool mShowingTitle		  = false;
	bool mShowingKeyboard	  = false;
	bool mInitialLoadError	  = false;
	bool mShowingVideo		  = false;
	bool mShowingWeb		  = false;
	bool mShowingWebcam		  = false;
	//std::string mMediaPropertyKey = "media";

	// 0 = normal, 1 = 90 degs, 2 = 180 degs, 3 = 270
	int mMediaRotation = 0;

	ds::time::Callback mPlayerLoadedTimer;
	ds::time::Callback mControlsTimeoutTimer;
};

} // namespace waffles
