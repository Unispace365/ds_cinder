#include "stdafx.h"

#include "video_volume_control.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/video.h>

namespace ds {
namespace ui {

VideoVolumeControl::VideoVolumeControl(ds::ui::SpriteEngine& eng, const float theSize, const float buttHeight, const ci::Color interfaceColor)
	: ds::ui::Sprite(eng, theSize * 1.5f, theSize)
	, mLinkedVideo(nullptr)
	, mOffOpacity(0.2f)
{
	//setTransparent(false);
	//setColor(ci::Color(0.5f, 0.0f, 0.0f));

	const int			barCount = 5;

	const float			floatNumBars = (float)(barCount);
	const float			gapPading = getWidth() / (floatNumBars * 3.0f);
	float				barWiddy = (getWidth() - (floatNumBars - 1.0f) * gapPading) / floatNumBars;
	float				xp = 0.0f;

	for(int k = 0; k < barCount; ++k) {
		ds::ui::Sprite*	s = new ds::ui::Sprite(mEngine, barWiddy, buttHeight * (float)(k + 1) / floatNumBars);
		if(!s) continue;
		s->setTransparent(false);
		s->setCenter(0.0f, 1.0f);
		s->setColor(interfaceColor);
		s->setPosition(xp, getHeight() / 2.0f + buttHeight / 2.0f);
		mBars.push_back(s);
		addChild(*s);

		xp += barWiddy + gapPading;
	}

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		if(ti.mFingerIndex != 0 || ti.mPhase == ds::ui::TouchInfo::Removed) return;
		float local_x = globalToLocal(ti.mCurrentGlobalPoint).x;
		if(local_x < 0.0f){
			setVolume(0.0f);
		} else if(local_x > getWidth()){
			setVolume(1.0f);
		} else {
			setVolume(local_x / getWidth());
		}
	});
}

void VideoVolumeControl::linkVideo(ds::ui::GstVideo* vid){
	mLinkedVideo = vid;
}

void VideoVolumeControl::setVolume(const float v){
	if(mLinkedVideo){
		if(mLinkedVideo->getIsMuted() && v > 0.0f){
			mLinkedVideo->setMute(false);
		}
		mLinkedVideo->setVolume(v);
	}
}

void VideoVolumeControl::onUpdateServer(const ds::UpdateParams& updateParams){
	float vol = 0.0f;
	if(mLinkedVideo){
		vol = mLinkedVideo->getVolume();
		if(mLinkedVideo->getIsMuted()){
			vol = 0.0f;
		}
	}
	for(int k = 0; k < mBars.size(); ++k) {
		const float		bar_v = static_cast<float>(k + 1) / static_cast<float>(mBars.size());
		if(vol >= bar_v) {
			mBars[k]->setOpacity(1.0f);
		} else {
			mBars[k]->setOpacity(mOffOpacity);
		}
	}
}

} // namespace ui
} // namespace ds
