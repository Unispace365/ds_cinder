#pragma once
#ifndef DS_UI_MEDIA_VIEWER_WEB_YOUTUBE_INTERFACE
#define DS_UI_MEDIA_VIEWER_WEB_YOUTUBE_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class ImageButton;
class Text;
class Web;
class LayoutSprite;
class SmartLayout;

/**
* \class WebInterface
*			Implements special interface for Youtube page forward/back and play
*/
class WebYoutubeInterface : public MediaInterface  {
public:
	WebYoutubeInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void				animateOff();

	void						linkWeb(ds::ui::Web* linkedWeb);
	void						updateWidgets();

	void						setAllowTouchToggle(const bool allowTouchToggling);
	void						playYutube();

	ds::ui::ImageButton*		getBackButton() { return mBackPageButton; }
	ds::ui::ImageButton*		getForwardButton() { return mForwardPageButton; }
	ds::ui::ImageButton*		getTouchToggleButton() { return mTouchToggle; }

protected:

	virtual void				onLayout();

	ds::ui::Web*				mLinkedWeb;

	bool						mAbleToTouchToggle;
	bool						mWebLocked;

	ds::ui::ImageButton*		mBackTimeButton;
	ds::ui::ImageButton*		mBackPageButton;
	ds::ui::ImageButton*		mForwardTimeButton;
	ds::ui::ImageButton*		mForwardPageButton;
	ds::ui::ImageButton*		mTouchToggle;
	ds::ui::ImageButton*		mPlayButton;

	ds::ui::SmartLayout*		mButtonHolder;
	//bool						mIsPlaying;
	bool						mIsFirstStart;

};

} // namespace ui
} // namespace ds

#endif
