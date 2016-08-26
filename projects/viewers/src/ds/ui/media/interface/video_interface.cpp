#include "video_interface.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/video.h>
#include <ds/ui/button/image_button.h>

#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"

namespace ds {
namespace ui {

VideoInterface::VideoInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& sizey, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor)
	: MediaInterface(eng, sizey, backgroundColor)
	, mLinkedVideo(nullptr)
	, mPlayButton(nullptr)
	, mPauseButton(nullptr)
	, mScrubBar(nullptr)
	, mVolumeControl(nullptr)
{



	mScrubBar = new VideoScrubBar(mEngine, sizey.y, buttonHeight, buttonColor);
	addChildPtr(mScrubBar);
	mVolumeControl = new VideoVolumeControl(mEngine, sizey.y, buttonHeight, buttonColor);
	addChildPtr(mVolumeControl);

	mPlayButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/play.png", "%APP%/data/images/media_interface/play_down.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mPlayButton);
	mPlayButton->setClickFn([this](){
		if(mLinkedVideo){
			mLinkedVideo->play();
		}
	});

	mPlayButton->getNormalImage().setColor(buttonColor);
	mPlayButton->getHighImage().setColor(buttonColor / 2.0f);
	mPlayButton->setScale(sizey.y / mPlayButton->getHeight());

	mPauseButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/pause.png", "%APP%/data/images/media_interface/pause_down.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mPauseButton);
	mPauseButton->setClickFn([this](){
		if(mLinkedVideo){
			mLinkedVideo->pause();
		}
	});

	mPauseButton->getNormalImage().setColor(buttonColor);
	mPauseButton->getHighImage().setColor(buttonColor / 2.0f);
	mPauseButton->setScale(sizey.y / mPauseButton->getHeight());

	const float padding = sizey.y / 2.0f; // config?
	mMinWidth = mPlayButton->getScaleWidth() + mVolumeControl->getScaleWidth() + padding * 3 + sizey.y * 4.0f; // last sizey is for the scrub bar
	mMaxWidth = 10000.0f; // WHOOOOOOOO

	layout();
}

void VideoInterface::linkVideo(ds::ui::GstVideo* vid){
	mLinkedVideo = vid;
	if(mVolumeControl){
		mVolumeControl->linkVideo(mLinkedVideo);
	}
	if(mScrubBar){
		mScrubBar->linkVideo(mLinkedVideo);
	}
}

void VideoInterface::updateServer(const ds::UpdateParams& p){
	MediaInterface::updateServer(p);

	if(mLinkedVideo && mPauseButton && mPlayButton){
		if(mLinkedVideo->getIsPlaying()){
			mPauseButton->show();
			mPlayButton->hide();
		} else {
			mPauseButton->hide();
			mPlayButton->show();
		}
	}
}


// Layout is called when the size is changed, so don't change the size in the layout
void VideoInterface::onLayout(){
	float w = getWidth();
	if(w < mMinWidth) w = mMinWidth;
	const float h = getHeight();
	const float padding = h / 2.0f; // config?
	float xp = getWidth() / 2.0f - w / 2.0f + padding;
	float spaceLeft = w - padding;
	if(mPlayButton && mPauseButton){
		mPlayButton->setPosition(xp, h / 2.0f - mPlayButton->getHeight() / 2.0f);
		mPauseButton->setPosition(xp, h / 2.0f - mPauseButton->getHeight() / 2.0f);
		xp += mPlayButton->getScaleWidth() + padding;
		spaceLeft -= mPlayButton->getScaleWidth() + padding;
	}

	if(mVolumeControl){
		mVolumeControl->setPosition(getWidth() / 2.0f + w / 2.0f - mVolumeControl->getWidth() - padding, h / 2.0f - mVolumeControl->getHeight() / 2.0f);
		spaceLeft -= mVolumeControl->getScaleWidth() + padding* 2.0f;
	}

	if(mScrubBar){
		mScrubBar->setSize(spaceLeft, mScrubBar->getHeight());
		mScrubBar->setPosition(xp, h / 2.0f - mScrubBar->getHeight() / 2.0f);
	}
}

} // namespace ui
} // namespace ds
