#pragma once

#include "ds/ui/media/media_interface.h"

namespace ds { namespace ui {

	class ImageButton;
	class Text;
	class YouTubeWeb;
	class LayoutSprite;
	class SmartLayout;
	class VideoScrubBar;
	class VideoVolumeControl;
	class LayoutButton;
	class ToggleContainer;

	/**
	 * \class YoutubeInterface
	 *			Implements special interface for Youtube page forward/back and play
	 */
	class YoutubeInterface : public MediaInterface {
	  public:
		YoutubeInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
						 const ci::Color buttonColor, const ci::Color backgroundColor);

		virtual void animateOff();

		virtual void onUpdateServer(const ds::UpdateParams& updateParams);
		void		 linkYouTubeWeb(ds::ui::YouTubeWeb* linkedWeb);
		void		 updateWidgets();

		void setAllowTouchToggle(const bool allowTouchToggling);

		ds::ui::LayoutButton* getBackButton();
		ds::ui::LayoutButton* getForwardButton();
		ds::ui::ToggleContainer* getTouchToggleButton();


		ds::ui::LayoutButton* getPlayButton();
		ds::ui::LayoutButton* getPauseButton();

		ds::ui::Sprite* getScrubBarBackground();
		ds::ui::Sprite* getScrubBarProgress();

		VideoVolumeControl* getVolumeControl();

	  protected:
		void		 setupButton(ds::ui::ImageButton* bs, ci::Color buttonColor, float height);
		virtual void onLayout();

		ds::ui::YouTubeWeb* mLinkedYouTube;

		bool mAbleToTouchToggle;
		bool mWebLocked;

		VideoScrubBar*		 mScrubBar;
		VideoVolumeControl*	 mVolumeControl;
		ds::ui::LayoutButton* mBackPageButton;
		ds::ui::LayoutButton* mForwardPageButton;
		ds::ui::ToggleContainer* mTouchToggle;
		ds::ui::LayoutButton* mPlayButton;
		ds::ui::LayoutButton* mPauseButton;
	};

}} // namespace ds::ui
