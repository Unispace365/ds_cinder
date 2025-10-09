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
					   const ci::Color& buttonColor, const ci::Color& backgroundColor);

		void linkVideo(ds::ui::GstVideo* linkedVideo);

		void onUpdateServer(const ds::UpdateParams& p) override;

		ds::ui::LayoutButton* getPlayButton() const;
		ds::ui::LayoutButton* getPauseButton() const;
		ds::ui::LayoutButton* getLoopButton() const;
		ds::ui::LayoutButton* getUnLoopButton() const;

		ds::ui::Sprite* getScrubBarBackground() const;
		ds::ui::Sprite* getScrubBarProgress() const;

		VideoVolumeControl* getVolumeControl() const;

		void addNubToScrubBar(ds::ui::Sprite* newNub) const;

	  protected:
		void onSizeLimitsChanged();
		void onLayout() override;

		ds::ui::GstVideo*	  mLinkedVideo;
		ds::ui::LayoutButton* mPlayButton;
		ds::ui::LayoutButton* mPauseButton;
		ds::ui::LayoutButton* mLoopButton;
		ds::ui::LayoutButton* mUnLoopButton;
		VideoScrubBar*		  mScrubBar;
		VideoVolumeControl*	  mVolumeControl;
		float				  mPadding;
	};

}} // namespace ds::ui

#endif
