#include "video_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/video.h>
#include <ds/ui/button/image_button.h>

#include "ds/ui/media/interface/video_interface.h"

namespace ds {
namespace ui {

VideoPlayer::VideoPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
	: ds::ui::Sprite(eng)
	, mVideo(nullptr)
	, mVideoInterface(nullptr)
	, mEmbedInterface(embedInterface)
{
}

void VideoPlayer::setMedia(const std::string mediaPath){
	if(mVideo){
		mVideo->release();
		mVideo = nullptr;
		if(mVideoInterface){
			mVideoInterface->linkVideo(nullptr);
		}
	}

	mVideo = new ds::ui::GstVideo(mEngine);
	mVideo->setLooping(true);

	mVideo->setMute(true);
	mVideo->setAutoStart(false);
	mVideo->loadVideo(mediaPath);
	mVideo->playAFrame(-1.0, [this](){
		mVideo->setMute(false);
	});
	addChildPtr(mVideo);

	if(!mVideoInterface){
		mVideoInterface = new VideoInterface(mEngine, ci::Vec2f(1050.0f, 50.0f), 25.0f, ci::Color::white(), ci::Color::black());
	}

	addChildPtr(mVideoInterface);
	mVideoInterface->sendToFront();

	// Leave the interface instance as a child, but hide it, in case something else want's to use it
	if(!mEmbedInterface){
		mVideoInterface->hide();
	}

	if(mVideoInterface && mVideo){
		mVideoInterface->linkVideo(mVideo);
	}

	setSize(mVideo->getWidth(), mVideo->getHeight());
}

void VideoPlayer::onSizeChanged(){
	layout();
}

void VideoPlayer::layout(){
	if(mVideo){
		mVideo->setScale(getWidth() / mVideo->getWidth());
	}

	if(mVideoInterface && mEmbedInterface){
		mVideoInterface->setSize(getWidth() / 2.0f, mVideoInterface->getHeight());
		mVideoInterface->setPosition(getWidth() / 2.0f - mVideoInterface->getWidth() / 2.0f, getHeight() - mVideoInterface->getHeight() - 50.0f);
	}
}

void VideoPlayer::showInterface(){
	if(mVideoInterface){
		mVideoInterface->animateOn();
	}
}

void VideoPlayer::play(){
	if(mVideo){
		if(mVideo->isPlayingAFrame()){
			mVideo->enablePlayingAFrame(false);
		}
		mVideo->play();
	}
}

void VideoPlayer::pause(){
	if(mVideo){
		mVideo->pause();
	}
}

void VideoPlayer::stop(){
	if(mVideo){
		mVideo->stop();
	}
}

MediaInterface* VideoPlayer::getExternalInterface(){
	return mVideoInterface;
}

} // namespace ui
} // namespace ds
