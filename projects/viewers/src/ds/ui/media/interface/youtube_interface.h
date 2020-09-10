#pragma once

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class YouTubeWeb;
class LayoutSprite;
class SmartLayout;
class VideoScrubBar;
class VideoVolumeControl;

/**
* \class YoutubeInterface
*			Implements special interface for Youtube page forward/back and play
*/
class YoutubeInterface : public MediaInterface  {
public:
	YoutubeInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void				animateOff();

	virtual void				onUpdateServer(const ds::UpdateParams& updateParams);
	void						linkYouTubeWeb(ds::ui::YouTubeWeb* linkedWeb);
	void						updateWidgets();

	void						setAllowTouchToggle(const bool allowTouchToggling);

	ds::ui::ImageButton*		getBackButton();
	ds::ui::ImageButton*		getForwardButton();
	ds::ui::ImageButton*		getTouchToggleButton();


	ds::ui::ImageButton*		getPlayButton();
	ds::ui::ImageButton*		getPauseButton();

	ds::ui::Sprite*				getScrubBarBackground();
	ds::ui::Sprite*				getScrubBarProgress();

	VideoVolumeControl*			getVolumeControl();

protected:

	void						setupButton(ds::ui::ImageButton* bs, ci::Color buttonColor, float height);
	virtual void				onLayout();

	ds::ui::YouTubeWeb*			mLinkedYouTube;

	bool						mAbleToTouchToggle;
	bool						mWebLocked;

	VideoScrubBar*				mScrubBar;
	VideoVolumeControl*			mVolumeControl;
	ds::ui::ImageButton*		mBackPageButton;
	ds::ui::ImageButton*		mForwardPageButton;
	ds::ui::ImageButton*		mTouchToggle;
	ds::ui::ImageButton*		mPlayButton;
	ds::ui::ImageButton*		mPauseButton;

};

} // namespace ui
} // namespace ds

