#include "media_interface.h"


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

MediaInterface::MediaInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& sizey, const ci::Color backgroudnColor)
	: ds::ui::Sprite(eng, sizey.x, sizey.y)
	, mBackground(nullptr)
	, mIdling(nullptr)
	, mAnimateDuration(0.35f)
{
	// TODO: settings?
	const float backOpacccy = 0.95f;
	const float idleSeconds = 5.0f;

	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->setTransparent(false);
	mBackground->setColor(backgroudnColor);
	mBackground->setOpacity(backOpacccy);
	addChildPtr(mBackground);

	setSecondBeforeIdle(idleSeconds);
	resetIdleTimer();
	layout();
}

void MediaInterface::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	if(mIdling != isIdling()){
		mIdling = isIdling();
		if(mIdling){
			animateOff();
		} else {
			animateOn();
		}
	}
}


// Layout is called when the size is changed, so don't change the size in the layout
void MediaInterface::layout(){
	const float w = getWidth();
	const float h = getHeight();
	if(mBackground){
		mBackground->setSize(w, h);
	}
	onLayout();
}

void MediaInterface::animateOn(){
	resetIdleTimer();
	show();

	float opacityDiff = (1.0f - getOpacity());
	if(opacityDiff > 0.0f){
		tweenOpacity(1.0f, mAnimateDuration * opacityDiff, 0.0f, ci::EaseNone());
	}
}

void MediaInterface::animateOff(){
	// TODO: settings
	tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this]{ hide(); });
}

void MediaInterface::turnOn(){
	resetIdleTimer();
	setOpacity(1.0f);
	show();
}

void MediaInterface::turnOff(){
	hide();
	setOpacity(0.0f);
}

void MediaInterface::onSizeChanged(){
	layout();
}

} // namespace ui
} // namespace ds
