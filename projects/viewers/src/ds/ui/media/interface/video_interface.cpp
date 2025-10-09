#include "stdafx.h"

#include "ds/ui/media/interface/video_interface.h"

#include "ds/ui/layout/smart_layout.h"
#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"

#include <ds/ui/button/layout_button.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/video.h>
#include <ds/util/string_util.h>


namespace ds::ui {

VideoInterface::VideoInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
							   const ci::Color& buttonColor, const ci::Color& backgroundColor)
  : MediaInterface(eng, ds::Resource::VIDEO_TYPE, interfaceSize, backgroundColor)
  , mLinkedVideo(nullptr)
  , mPlayButton(nullptr)
  , mPauseButton(nullptr)
  , mLoopButton(nullptr)
  , mUnLoopButton(nullptr)
  , mScrubBar(nullptr)
  , mVolumeControl(nullptr) {


	mScrubBar = new VideoScrubBar(mEngine, interfaceSize.y, mScrubBarHeight, buttonColor);
	addChildPtr(mScrubBar);
	mVolumeControl = new VideoVolumeControl(mEngine, interfaceSize.y, buttonHeight, buttonColor);

	mVolumeControl->setSliderHeight(mVolumeSliderHeight);
	mVolumeControl->setNubSize(mVolumeSliderHeight * 1.5f);
	addChildPtr(mVolumeControl);


	mPlayButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:play:normal:file",
							   "ui:media_button:play:pressed:file");
	addChildPtr(mPlayButton);
	mPlayButton->setClickFn([this]() {
		if (mLinkedVideo) {
			mLinkedVideo->play();
		}
	});
	mPlayButton->setDimensionsChangedCallback([this](ds::ui::Sprite*) { onSizeLimitsChanged(); });
	MediaInterface::setButtonColor(mPlayButton, buttonColor, buttonColor * 0.5f);


	mPauseButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:pause:normal:file",
								"ui:media_button:pause:pressed:file");
	addChildPtr(mPauseButton);
	mPauseButton->setClickFn([this]() {
		if (mLinkedVideo) {
			mLinkedVideo->pause();
		}
	});
	mPauseButton->setDimensionsChangedCallback([this](ds::ui::Sprite*) { onSizeLimitsChanged(); });
	MediaInterface::setButtonColor(mPauseButton, buttonColor, buttonColor * 0.5f);


	mLoopButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:loop:normal:file",
							   "ui:media_button:loop:pressed:file");
	addChildPtr(mLoopButton);
	mLoopButton->setClickFn([this]() {
		if (mLinkedVideo) {
			mLinkedVideo->setLooping(false);
		}
	});
	mLoopButton->setDimensionsChangedCallback([this](ds::ui::Sprite*) { onSizeLimitsChanged(); });
	MediaInterface::setButtonColor(mLoopButton, buttonColor, buttonColor * 0.5f);


	mUnLoopButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:unloop:normal:file",
								 "ui:media_button:unloop:pressed:file");
	addChildPtr(mUnLoopButton);
	mUnLoopButton->setClickFn([this]() {
		if (mLinkedVideo) {
			mLinkedVideo->setLooping(true);
		}
	});
	mUnLoopButton->setDimensionsChangedCallback([this](ds::ui::Sprite*) { onSizeLimitsChanged(); });
	MediaInterface::setButtonColor(mUnLoopButton, buttonColor, buttonColor * 0.5f);


	mPadding = interfaceSize.y / 1.5f; // config?

	onSizeLimitsChanged();

	layout();
}

void VideoInterface::linkVideo(ds::ui::GstVideo* linkedVideo) {
	mLinkedVideo = linkedVideo;
	if (mVolumeControl) {
		mVolumeControl->linkVideo(mLinkedVideo);
	}
	if (mScrubBar) {
		mScrubBar->linkVideo(mLinkedVideo);
	}
}

ds::ui::LayoutButton* VideoInterface::getPlayButton() const {
	return mPlayButton;
}

ds::ui::LayoutButton* VideoInterface::getPauseButton() const {
	return mPauseButton;
}

ds::ui::LayoutButton* VideoInterface::getLoopButton() const {
	return mLoopButton;
}

ds::ui::LayoutButton* VideoInterface::getUnLoopButton() const {
	return mUnLoopButton;
}

ds::ui::Sprite* VideoInterface::getScrubBarBackground() const {
	if (!mScrubBar) return nullptr;
	return mScrubBar->getBacker();
}

ds::ui::Sprite* VideoInterface::getScrubBarProgress() const {
	if (!mScrubBar) return nullptr;
	return mScrubBar->getProgress();
}

ds::ui::VideoVolumeControl* VideoInterface::getVolumeControl() const {
	return mVolumeControl;
}

void VideoInterface::addNubToScrubBar(ds::ui::Sprite* newNub) const {
	if (!mScrubBar) return;
	mScrubBar->addNub(newNub);
}

void VideoInterface::onSizeLimitsChanged() {
	const float scrubWidth =
		mScrubBar->getHeight() * 2.0f; // used to be 4x, but this will allow for smaller controllers
	mMinWidth = mPlayButton->getScaleWidth() + mLoopButton->getScaleWidth() + mVolumeControl->getScaleWidth() +
				scrubWidth + mPadding * 5;
	mMaxWidth = 10000.0f;

	if (getWidth() < mMinWidth || getWidth() > mMaxWidth) setSize(getWidth(), getHeight());
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
	const float width  = getWidth();
	const float height = getHeight();

	// Streaming videos only have the volume control thingy
	if (mLinkedVideo && mLinkedVideo->getIsStreaming() && mVolumeControl) {
		mVolumeControl->setPosition((width - mVolumeControl->getWidth()) * 0.5f,
									(height - mVolumeControl->getHeight()) * 0.5f);
		return;
	}

	// Measure the size of all elements
	float totalWidth = 0.0f;
	if (mPlayButton) {
		totalWidth += mPlayButton->getScaleWidth();
	} else if (mPauseButton) {
		totalWidth += mPauseButton->getScaleWidth();
	}
	if (mLoopButton) {
		if (totalWidth > 0) totalWidth += mPadding;
		totalWidth += mLoopButton->getScaleWidth();
	} else if (mUnLoopButton) {
		if (totalWidth > 0) totalWidth += mPadding;
		totalWidth += mUnLoopButton->getScaleWidth();
	}
	if (mVolumeControl) {
		if (totalWidth > 0) totalWidth += mPadding;
		totalWidth += mVolumeControl->getScaleWidth();
	}
	if (mScrubBar) {
		if (totalWidth > 0) totalWidth += mPadding;
		totalWidth += mScrubBar->getScaleWidth();
	}

	// Adjust the size of the scrub bar.
	float excessWidth = totalWidth - width;
	if (mScrubBar) {
		mScrubBar->setSize(glm::max(0.0f, mScrubBar->getScaleWidth() - excessWidth), mScrubBar->getScaleHeight());
		mScrubBar->setScale(1);
		totalWidth -= excessWidth;
	}

	// Position elements.
	float position = 0.5f * (width - totalWidth);

	if (mPlayButton && mPauseButton) {
		mPlayButton->setPosition(position, (height - mPlayButton->getScaleHeight()) * 0.5f);
		mPauseButton->setPosition(position, (height - mPauseButton->getScaleHeight()) * 0.5f);
		position += mPlayButton->getScaleWidth() + mPadding;
	}

	if (mScrubBar) {
		mScrubBar->setPosition(position, (height - mScrubBar->getScaleHeight()) * 0.5f);
		position += mScrubBar->getScaleWidth() + mPadding;
	}

	if (mLoopButton && mUnLoopButton) {
		mLoopButton->setPosition(position, (height - mLoopButton->getScaleHeight()) * 0.5f);
		mUnLoopButton->setPosition(position, (height - mUnLoopButton->getScaleHeight()) * 0.5f);
		position += mLoopButton->getScaleWidth() + mPadding;
	}

	if (mVolumeControl) {
		mVolumeControl->setPosition(position, (height - mVolumeControl->getScaleHeight()) * 0.5f);
		position += mVolumeControl->getScaleWidth() + mPadding;
	}
}

} // namespace ds::ui