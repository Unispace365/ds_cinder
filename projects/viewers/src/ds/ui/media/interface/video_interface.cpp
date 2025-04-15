#include "stdafx.h"

#include "video_interface.h"


#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/video.h>

#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"

namespace ds { namespace ui {

	VideoInterface::VideoInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const float buttonHeight,
								   const ci::Color buttonColor, const ci::Color backgroundColor)
	  : MediaInterface(eng, ds::Resource::VIDEO_TYPE, sizey, backgroundColor)
	  , mLinkedVideo(nullptr)
	  , mPlayButton(nullptr)
	  , mPauseButton(nullptr)
	  , mLoopButton(nullptr)
	  , mUnLoopButton(nullptr)
	  , mScrubBar(nullptr)
	  , mVolumeControl(nullptr) {


		mScrubBar = new VideoScrubBar(mEngine, sizey.y, mScrubBarHeight, buttonColor);
		addChildPtr(mScrubBar);
		mVolumeControl = new VideoVolumeControl(mEngine, sizey.y, buttonHeight, buttonColor);
		
		mVolumeControl->setSliderHeight(mVolumeSliderHeight);
		mVolumeControl->setNubSize(mVolumeSliderHeight * 1.5);
		addChildPtr(mVolumeControl);

		
		mPlayButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:play:normal:file",
								   "ui:media_button:play:pressed:file");
		addChildPtr(mPlayButton);
		mPlayButton->setClickFn([this]() {
			if (mLinkedVideo) {
				mLinkedVideo->play();
			}
		});
		setButtonColor(mPlayButton, buttonColor, buttonColor / 2.0f);
				
		mPauseButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:pause:normal:file",
									"ui:media_button:pause:pressed:file");
		addChildPtr(mPauseButton);
		mPauseButton->setClickFn([this]() {
			if (mLinkedVideo) {
				mLinkedVideo->pause();
			}
		});
		setButtonColor(mPauseButton, buttonColor, buttonColor / 2.0f);

		
		mLoopButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:loop:normal:file","ui:media_button:loop:pressed:file");
		addChildPtr(mLoopButton);
		mLoopButton->setClickFn([this]() {
			if (mLinkedVideo) {
				mLinkedVideo->setLooping(false);
			}
		});
		setButtonColor(mLoopButton, buttonColor, buttonColor / 2.0f);
		
		mUnLoopButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:unloop:normal:file","ui:media_button:unloop:pressed:file");
		addChildPtr(mUnLoopButton);
		mUnLoopButton->setClickFn([this]() {
			if (mLinkedVideo) {
				mLinkedVideo->setLooping(true);
			}
		});
		setButtonColor(mUnLoopButton, buttonColor, buttonColor / 2.0f);
		

		const float padding = sizey.y / 1.5f; // config?
		mMinWidth = mPlayButton->getScaleWidth() + mLoopButton->getScaleWidth() + mVolumeControl->getScaleWidth() + padding * 2.f +
					padding * 4 + sizey.y * 4.0f; // last sizey is for the scrub bar
		mMaxWidth = 10000.0f;					  // WHOOOOOOOO

		layout();
	}

	void VideoInterface::linkVideo(ds::ui::GstVideo* vid) {
		mLinkedVideo = vid;
		if (mVolumeControl) {
			mVolumeControl->linkVideo(mLinkedVideo);
		}
		if (mScrubBar) {
			mScrubBar->linkVideo(mLinkedVideo);
		}
	}

	ds::ui::LayoutButton* VideoInterface::getPlayButton() {
		return mPlayButton;
	}

	ds::ui::LayoutButton* VideoInterface::getPauseButton() {
		return mPauseButton;
	}

	ds::ui::LayoutButton* VideoInterface::getLoopButton() {
		return mLoopButton;
	}

	ds::ui::LayoutButton* VideoInterface::getUnLoopButton() {
		return mUnLoopButton;
	}

	ds::ui::Sprite* VideoInterface::getScrubBarBackground() {
		if (!mScrubBar) return nullptr;
		return mScrubBar->getBacker();
	}

	ds::ui::Sprite* VideoInterface::getScrubBarProgress() {
		if (!mScrubBar) return nullptr;
		return mScrubBar->getProgress();
	}

	ds::ui::VideoVolumeControl* VideoInterface::getVolumeControl() {
		return mVolumeControl;
	}

	void VideoInterface::addNubToScrubBar(ds::ui::Sprite* newNub) {
		if (!mScrubBar) return;
		mScrubBar->addNub(newNub);
	}

	void VideoInterface::onUpdateServer(const ds::UpdateParams& p) {
		MediaInterface::onUpdateServer(p);

		if (mLinkedVideo && mPauseButton && mPlayButton && mLoopButton && mUnLoopButton) {
			if (mLinkedVideo->getIsStreaming()) {
				mPlayButton->hide();
				mPauseButton->hide();
				mUnLoopButton->hide();
				mLoopButton->hide();

			} else {
				if (mLinkedVideo->getIsPlaying()) {
					mPauseButton->show();
					mPlayButton->hide();
				} else {
					mPauseButton->hide();
					mPlayButton->show();
				}

				if (mLinkedVideo->getIsLooping()) {
					mUnLoopButton->hide();
					mLoopButton->show();
				} else {
					mUnLoopButton->show();
					mLoopButton->hide();
				}
			}
		}
	}


	// Layout is called when the size is changed, so don't change the size in the layout
	void VideoInterface::onLayout() {

		float w = glm::clamp(getWidth(), mMinWidth, mMaxWidth);
		// if (w < mMinWidth) w = mMinWidth;
		const float h		  = getHeight();
		const float padding	  = h / 1.5f; // config?
		float		xp		  = getWidth() / 2.0f - w / 2.0f + padding;
		float		spaceLeft = w - padding * 2.f;

		// Streaming videos only have the volume control thingy
		if (mLinkedVideo && mLinkedVideo->getIsStreaming() && mVolumeControl) {
			mVolumeControl->setPosition(getWidth() / 2.0f - mVolumeControl->getWidth() / 2.0f,
										h / 2.0f - mVolumeControl->getHeight() / 2.0f);
			return;
		}

		if (mPlayButton && mPauseButton) {
			mPlayButton->setPosition(xp, h / 2.0f - mPlayButton->getScaleHeight() / 2.0f);
			mPauseButton->setPosition(xp, h / 2.0f - mPauseButton->getScaleHeight() / 2.0f);
			xp += mPlayButton->getScaleWidth() + padding;
			spaceLeft -= mPlayButton->getScaleWidth() + padding;
		}

		if (mVolumeControl) {
			auto gw = getWidth();
			auto gw2 = gw / 2.0f;
			auto w2	 = w / 2.0f;

			mVolumeControl->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getScaleWidth() - padding * 0.5f,
										h / 2.0f - mVolumeControl->getScaleHeight() / 2.0f);
			spaceLeft -= mVolumeControl->getScaleWidth() + padding * 1.0f;
		}

		if (mLoopButton && mUnLoopButton && mVolumeControl) {
			mLoopButton->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getScaleWidth() - padding * 2.5f -
										 mLoopButton->getScaleWidth(),
									 h / 2.0f - mLoopButton->getScaleHeight() / 2.0f);
			mUnLoopButton->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getScaleWidth() - padding * 2.5f -
										   mUnLoopButton->getScaleWidth(),
									   h / 2.0f - mUnLoopButton->getScaleHeight() / 2.0f);
			spaceLeft -= mLoopButton->getScaleWidth() + padding;
		}

		if (mScrubBar) {
			mScrubBar->setSize(spaceLeft, mScrubBar->getHeight());
			mScrubBar->setPosition(xp, h / 2.0f - mScrubBar->getHeight() / 2.0f);
		}
	}

}} // namespace ds::ui
