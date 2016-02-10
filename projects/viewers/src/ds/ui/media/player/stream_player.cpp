#include "stream_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/video.h>
#include <ds/ui/button/image_button.h>

#include "ds/ui/media/interface/video_interface.h"
#include "ds/ui/media/media_interface_builder.h"

namespace ds {
namespace ui {

StreamPlayer::StreamPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
	: ds::ui::Sprite(eng)
	, mVideo(nullptr)
	, mVideoInterface(nullptr)
	, mEmbedInterface(embedInterface)
	, mShowInterfaceAtStart(true)
{
	}

void StreamPlayer::setResource(const ds::Resource& resource){

	if (mVideo){
		mVideo->release();
		mVideo = nullptr;
		if (mVideoInterface){
			mVideoInterface->linkVideo(nullptr);
		}
	}

	mVideo = new ds::ui::GstVideo(mEngine);
	mVideo->generateAudioBuffer(true);
	mVideo->setLooping(true);

	mVideo->setResource(resource);

	addChildPtr(mVideo);

	if (mVideoInterface){
		mVideoInterface->release();
		mVideoInterface = nullptr;
	}

	if (mEmbedInterface){
		mVideoInterface = dynamic_cast<VideoInterface*>(MediaInterfaceBuilder::buildMediaInterface(mEngine, this, this));
		if (mVideoInterface){
			mVideoInterface->sendToFront();
		}
	}

	if (mVideoInterface){
		if (mShowInterfaceAtStart){
			mVideoInterface->show();
		}
		else {
			mVideoInterface->setOpacity(0.0f);
			mVideoInterface->hide();
		}

		if (resource.getType() == ds::Resource::VIDEO_STREAM_TYPE){
			mVideoInterface->hide();
		}

	}

	setSize(mVideo->getWidth(), mVideo->getHeight());

}


void StreamPlayer::onSizeChanged(){
	layout();
}

void StreamPlayer::layout(){
	if (mVideo){
		mVideo->setScale(getWidth() / mVideo->getWidth());
	}

	if (mVideoInterface && mEmbedInterface){
		mVideoInterface->setSize(getWidth() / 2.0f, mVideoInterface->getHeight());
		mVideoInterface->setPosition(getWidth() / 2.0f - mVideoInterface->getWidth() / 2.0f, getHeight() - mVideoInterface->getHeight() - 50.0f);
	}
}

void StreamPlayer::showInterface(){
	if (mVideoInterface){
		mVideoInterface->animateOn();
	}
}

void StreamPlayer::setShowInterfaceAtStart(bool showInterfaceAtStart){
	mShowInterfaceAtStart = showInterfaceAtStart;
}

void StreamPlayer::setAutoRestartStream(bool autoRestart){
	if(mVideo){
		mVideo->setAutoRestartStream(autoRestart);
	}
}

void StreamPlayer::play(){
	if (mVideo){
		if (mVideo->isPlayingAFrame()){
			mVideo->enablePlayingAFrame(false);
			mVideo->setMute(false);
		}
		mVideo->play();
	}
}

void StreamPlayer::pause(){
	if (mVideo){
		mVideo->pause();
	}
}

void StreamPlayer::stop(){
	if (mVideo){
		mVideo->stop();
	}
}

ds::ui::GstVideo* StreamPlayer::getVideo(){
	return mVideo;
}

} // namespace ui
} // namespace ds
