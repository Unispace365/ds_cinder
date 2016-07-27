#include "video_scrub_bar.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/video.h>

namespace ds {
namespace ui {

VideoScrubBar::VideoScrubBar(ds::ui::SpriteEngine& eng, const float heighty, const float buttHeight, const ci::Color interfaceColor)
	: ds::ui::Sprite(eng)
	, mBacker(nullptr)
	, mProgress(nullptr)
	, mLinkedVideo(nullptr)
{

	// 	setTransparent(false);
	// 	setColor(ci::Color(0.0f, 0.5f, 0.0f));

	const float widdyWamWamWozzle = 400.0f;

	setSize(widdyWamWamWozzle, heighty);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){
		// seek to the relative position, or something
		if(!getParent() || !mLinkedVideo) return;
		ci::Vec3f loccy = globalToLocal(ti.mCurrentGlobalPoint);
		double newPercent = (double)(loccy.x / getWidth());
		if(newPercent < 0.0) newPercent = 0.0;
		if(newPercent > 1.0) newPercent = 1.0;

		mLinkedVideo->seekPosition(newPercent);
	});

	mBacker = new ds::ui::Sprite(mEngine, widdyWamWamWozzle, buttHeight / 2.0f);
	mBacker->setTransparent(false);
	mBacker->setColor(interfaceColor);
	mBacker->enable(false);
	mBacker->setCenter(0.0f, 0.5f);
	mBacker->setPosition(0.0f, getHeight() / 2.0f);
	mBacker->setOpacity(0.25f);
	addChild(*mBacker);

	mProgress = new ds::ui::Sprite(mEngine, 0.0f, buttHeight / 2.0f);
	mProgress->setTransparent(false);
	mProgress->setColor(interfaceColor);
	mProgress->enable(false);
	mProgress->setCenter(0.0f, 0.5f);
	mProgress->setPosition(0.0f, getHeight() / 2.0f);
	addChild(*mProgress);

}

void VideoScrubBar::linkVideo(ds::ui::GstVideo* vid){
	mLinkedVideo = vid;
}

void VideoScrubBar::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	if(mLinkedVideo && mProgress){
		// update scrub bar
		float progress = (float)mLinkedVideo->getCurrentPosition();
		if(progress < 0.0f) progress = 0.0f;
		if(progress > 1.0f) progress = 1.0f;
		mProgress->setSize(progress * getWidth(), mProgress->getHeight());
	}
}

// don't change the size in layout
void VideoScrubBar::layout(){
	if(mBacker){
		mBacker->setSize(getWidth(), mBacker->getHeight());
	}
}

void VideoScrubBar::onSizeChanged(){
	layout();
}

} // namespace ui
} // namespace ds
