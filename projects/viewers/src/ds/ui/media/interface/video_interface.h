#pragma once
#ifndef DS_UI_MEDIA_VIEWER_VIDEO_INTERFACE
#define DS_UI_MEDIA_VIEWER_VIDEO_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds { namespace ui {

	class GstVideo;
	class ImageButton;
	class VideoScrubBar;
	class VideoVolumeControl;

	/**
	 * \class VideoInterface
	 *			Implements play/pause, scrub bar, volume control
	 */
	class VideoInterface : public MediaInterface {
	  public:
		VideoInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
					   const ci::Color buttonColor, const ci::Color backgroundColor);

		void linkVideo(ds::ui::GstVideo* linkedVideo);

		virtual void onUpdateServer(const ds::UpdateParams& p) override;

		ds::ui::LayoutButton* getPlayButton();
		ds::ui::LayoutButton* getPauseButton();
		ds::ui::LayoutButton* getLoopButton();
		ds::ui::LayoutButton* getUnLoopButton();

		ds::ui::Sprite* getScrubBarBackground();
		ds::ui::Sprite* getScrubBarProgress();

		VideoVolumeControl* getVolumeControl();

		void addNubToScrubBar(ds::ui::Sprite* newNub);

	  protected:
		virtual void onLayout();

		ds::ui::GstVideo* mLinkedVideo;

		ds::ui::LayoutButton* mPlayButton;
		ds::ui::LayoutButton* mPauseButton;

		ds::ui::LayoutButton* mLoopButton;
		ds::ui::LayoutButton* mUnLoopButton;

		VideoScrubBar*		mScrubBar;
		VideoVolumeControl* mVolumeControl;
	};

}} // namespace ds::ui

#endif
