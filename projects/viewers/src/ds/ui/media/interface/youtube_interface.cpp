#include "stdafx.h"

#include "youtube_interface.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/web.h>

#include <ds/ui/layout/layout_sprite.h>


#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"
#include <ds/ui/media/player/youtube_player.h>

namespace ds { namespace ui {

	YoutubeInterface::YoutubeInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const float buttonHeight,
									   const ci::Color buttonColor, const ci::Color backgroundColor)
	  : MediaInterface(eng, sizey, backgroundColor)
	  , mLinkedYouTube(nullptr)
	  , mPlayButton(nullptr)
	  , mPauseButton(nullptr)
	  , mScrubBar(nullptr)
	  , mVolumeControl(nullptr)
	  , mBackPageButton(nullptr)
	  , mForwardPageButton(nullptr)
	  , mTouchToggle(nullptr)
	  , mAbleToTouchToggle(false)
	  , mWebLocked(false) {

		enable(true);

		mScrubBar = new VideoScrubBar(mEngine, sizey.y, buttonHeight, buttonColor);
		addChildPtr(mScrubBar);
		mVolumeControl = new VideoVolumeControl(mEngine, sizey.y, buttonHeight, buttonColor);
		addChildPtr(mVolumeControl);

		mPlayButton =
			new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/play.png",
									"%APP%/data/images/media_interface/play_down.png", (sizey.y - buttonHeight) / 2.0f);
		addChildPtr(mPlayButton);
		mPlayButton->setClickFn([this]() {
			if (mLinkedYouTube) {
				mLinkedYouTube->play();
			}
		});

		mPlayButton->getNormalImage().setColor(buttonColor);
		mPlayButton->getHighImage().setColor(buttonColor / 2.0f);
		mPlayButton->setScale(sizey.y / mPlayButton->getHeight());

		mPauseButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/pause.png",
											   "%APP%/data/images/media_interface/pause_down.png",
											   (sizey.y - buttonHeight) / 2.0f);
		addChildPtr(mPauseButton);
		mPauseButton->setClickFn([this]() {
			if (mLinkedYouTube) {
				mLinkedYouTube->pause();
			}
		});

		mPauseButton->getNormalImage().setColor(buttonColor);
		mPauseButton->getHighImage().setColor(buttonColor / 2.0f);
		mPauseButton->setScale(sizey.y / mPauseButton->getHeight());

		/*
		mBackPageButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/prev.png",
		"%APP%/data/images/media_interface/prev.png", (sizey.y - buttonHeight) / 2.0f); setupButton(mBackPageButton,
		buttonColor, sizey.y); mBackPageButton->setClickFn([this]() { if (mLinkedYouTube) { mLinkedYouTube->goBack();
				updateWidgets();
			}
		});


		mForwardPageButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/next.png",
		"%APP%/data/images/media_interface/next.png", (sizey.y - buttonHeight) / 2.0f); setupButton(mForwardPageButton,
		buttonColor, sizey.y); mForwardPageButton->setClickFn([this]() { if (mLinkedYouTube) {
				mLinkedYouTube->goForward();
				updateWidgets();
			}
		});

		mTouchToggle = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/touch_unlocked.png",
		"%APP%/data/images/media_interface/touch_unlocked.png", (sizey.y - buttonHeight) / 2.0f);
		setupButton(mTouchToggle, buttonColor, sizey.y);
		mTouchToggle->setClickFn([this]() {
			if (mLinkedYouTube) {
				if (mLinkedYouTube->isEnabled()) {
					mLinkedYouTube->enable(false);
				} else {
					mLinkedYouTube->enable(true);
				}
				updateWidgets();
			}
		});

		if (mAbleToTouchToggle) {
			mTouchToggle->show();
		} else {
			mTouchToggle->hide();
		}
		*/

		const float padding = sizey.y / 4.0f;

		mMinWidth = (padding * 4.0f + mVolumeControl->getScaleWidth() + padding +
					 // mBackPageButton->getScaleWidth() + padding +
					 // mForwardPageButton->getScaleWidth() + padding +
					 mPlayButton->getScaleWidth() + padding +
					 // mTouchToggle->getScaleWidth() + padding * 2.0f +
					 sizey.y * 4.0f // for scrub bar
		);
		mMaxWidth = 10000.0f; // WHOOOOOOOO

		updateWidgets();
	}


	void YoutubeInterface::setupButton(ds::ui::ImageButton* bs, ci::Color buttonColor, float height) {
		bs->getNormalImage().setColor(buttonColor);
		bs->getHighImage().setColor(buttonColor / 2.0f);
		bs->setScale(height / bs->getHeight());
		addChildPtr(bs);
	}

	void YoutubeInterface::setAllowTouchToggle(const bool allowTouchToggling) {
		if (allowTouchToggling) {
			DS_LOG_WARNING("YouTubeInterface: touch toggling is currently not allowed");
		}
		return;
		mAbleToTouchToggle = allowTouchToggling;
		if (mTouchToggle) {
			if (mAbleToTouchToggle) {
				mTouchToggle->show();
			} else {
				mTouchToggle->hide();
			}

			layout();
		}
	}


	ds::ui::ImageButton* YoutubeInterface::getBackButton() {
		return mBackPageButton;
	}

	ds::ui::ImageButton* YoutubeInterface::getForwardButton() {
		return mForwardPageButton;
	}

	ds::ui::ImageButton* YoutubeInterface::getTouchToggleButton() {
		return mTouchToggle;
	}

	ds::ui::ImageButton* YoutubeInterface::getPlayButton() {
		return mPlayButton;
	}

	ds::ui::ImageButton* YoutubeInterface::getPauseButton() {
		return mPauseButton;
	}

	ds::ui::Sprite* YoutubeInterface::getScrubBarBackground() {
		if (!mScrubBar) return nullptr;
		return mScrubBar->getBacker();
	}

	ds::ui::Sprite* YoutubeInterface::getScrubBarProgress() {
		if (!mScrubBar) return nullptr;
		return mScrubBar->getProgress();
	}

	ds::ui::VideoVolumeControl* YoutubeInterface::getVolumeControl() {
		return mVolumeControl;
	}


	void YoutubeInterface::animateOff() {
		mIdling = false;

		tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this] { hide(); });
	}


	void YoutubeInterface::onUpdateServer(const ds::UpdateParams& p) {
		MediaInterface::onUpdateServer(p);
		updateWidgets();
	}

	void YoutubeInterface::linkYouTubeWeb(ds::ui::YouTubeWeb* linkedWeb) {
		if (mLinkedYouTube) {
			mLinkedYouTube->setAuthCallback(nullptr);
			mLinkedYouTube->setLoadingUpdatedCallback(nullptr);
		}

		mLinkedYouTube = linkedWeb;

		if (mVolumeControl) {
			mVolumeControl->linkYouTube(mLinkedYouTube);
		}
		if (mScrubBar) {
			mScrubBar->linkYouTube(mLinkedYouTube);
		}

		updateWidgets();
	}

	// TODO: make this into a layoutsprite
	// Layout is called when the size is changed, so don't change the size in the layout
	void YoutubeInterface::onLayout() {
		const float w = getWidth();
		const float h = getHeight();
		if (mScrubBar && mVolumeControl && mPlayButton) {
			float w = getWidth();
			if (w < mMinWidth) w = mMinWidth;
			const float h		  = getHeight();
			const float padding	  = h / 2.0f; // config?
			float		xp		  = getWidth() / 2.0f - w / 2.0f + padding;
			float		spaceLeft = w - padding;


			if (mPlayButton && mPauseButton) {
				mPlayButton->setPosition(xp, h / 2.0f - mPlayButton->getScaleHeight() / 2.0f);
				mPauseButton->setPosition(xp, h / 2.0f - mPauseButton->getScaleHeight() / 2.0f);
				xp += mPlayButton->getScaleWidth() + padding;
				spaceLeft -= mPlayButton->getScaleWidth() + padding;
			}

			if (mVolumeControl) {
				mVolumeControl->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getWidth() - padding,
											h / 2.0f - mVolumeControl->getHeight() / 2.0f);
				spaceLeft -= mVolumeControl->getScaleWidth() + padding * 2.0f;
			}

			/*
			if (mForwardPageButton && mBackPageButton && mVolumeControl) {
				mForwardPageButton->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getWidth() - padding
			* 2.0f - mForwardPageButton->getScaleWidth(), h / 2.0f - mForwardPageButton->getScaleHeight() / 2.0f);
				mBackPageButton->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getWidth() - padding * 2.0f
			- mBackPageButton->getScaleWidth(), h / 2.0f - mBackPageButton->getScaleHeight() / 2.0f); spaceLeft -=
			mBackPageButton->getScaleWidth() + padding;
			}
			*/

			if (mScrubBar) {
				mScrubBar->setSize(spaceLeft, mScrubBar->getHeight());
				mScrubBar->setPosition(xp, h / 2.0f - mScrubBar->getHeight() / 2.0f);
			}
		}
	}

	void YoutubeInterface::updateWidgets() {
		if (mScrubBar && mVolumeControl && mPlayButton && mLinkedYouTube) {
			// TODO: settings / config for disabled opacity / color

			if (mLinkedYouTube->getIsPlaying()) {
				mPauseButton->show();
				mPlayButton->hide();
			} else {
				mPauseButton->hide();
				mPlayButton->show();
			}

			if (mBackPageButton) {
				if (mLinkedYouTube->canGoBack()) {
					mBackPageButton->enable(true);
					mBackPageButton->setOpacity(1.0f);
				} else {
					mBackPageButton->enable(false);
					mBackPageButton->setOpacity(0.25f);
				}
			}

			if (mForwardPageButton) {
				if (mLinkedYouTube->canGoForward()) {
					mForwardPageButton->enable(true);
					mForwardPageButton->setOpacity(1.0f);
				} else {
					mForwardPageButton->enable(false);
					mForwardPageButton->setOpacity(0.25f);
				}
			}
			if (mTouchToggle) {
				if (mLinkedYouTube->isEnabled()) {
					if (!mWebLocked) {
						mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png",
																  ds::ui::Image::IMG_CACHE_F);
						mTouchToggle->getNormalImage().setImageFile(
							"%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
						mWebLocked = true;
						mLinkedYouTube->setAllowClicks(true);
					}
				} else {
					if (mWebLocked) {
						mTouchToggle->getHighImage().setImageFile(
							"%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
						mTouchToggle->getNormalImage().setImageFile(
							"%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
						mWebLocked = false;
						mLinkedYouTube->setAllowClicks(false);
					}
				}
			}
		}

		layout();
	}

}} // namespace ds::ui
