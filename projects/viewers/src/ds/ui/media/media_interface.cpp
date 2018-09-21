#include "stdafx.h"

#include "media_interface.h"


#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/video.h>

#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"

namespace ds {
namespace ui {

MediaInterface::MediaInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const ci::Color backgroudnColor)
  : ds::ui::Sprite(eng, sizey.x, sizey.y)
  , mBackground(nullptr)
  , mIdling(nullptr)
  , mAnimateDuration(0.35f)
  , mMaxWidth(sizey.x)
  , mMinWidth(sizey.y)
  , mInterfaceIdleSettings(5.0f)
  , mCanIdle(true)
  , mCanDisplay(true) {
	// TODO: settings?
	const float backOpacccy = 0.95f;

	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->setTransparent(false);
	mBackground->setColor(backgroudnColor);
	mBackground->setOpacity(backOpacccy);
	addChildPtr(mBackground);

	setSecondBeforeIdle(mInterfaceIdleSettings);
	resetIdleTimer();
	layout();
}

void MediaInterface::onUpdateServer(const ds::UpdateParams& p) {
	if (mCanIdle && mIdling != isIdling()) {
		mIdling = isIdling();
		if (mIdling) {
			animateOff();
		} else {
			animateOn();
		}
	}
}

// Layout is called when the size is changed, so don't change the size in the layout
void MediaInterface::layout() {
	const float w = getWidth();
	const float h = getHeight();
	onLayout();
	if (mBackground) {
		float newW = w;
		if (newW < mMinWidth) newW = mMinWidth;
		if (newW > mMaxWidth) newW = mMaxWidth;

		mBackground->setSize(newW, h);
		mBackground->setCenter(0.5f, 0.0f);
		mBackground->setPosition(w / 2.0f, 0.0f);
	}
}


void MediaInterface::setBackgroundColorA(const ci::ColorA backgroundColor) {
	if (mBackground) mBackground->setColorA(backgroundColor);
}

void MediaInterface::animateOn() {
	if (!mCanDisplay) return;
	resetIdleTimer();
	show();

	float opacityDiff = (1.0f - getOpacity());
	if (opacityDiff > 0.0f) {
		tweenOpacity(1.0f, mAnimateDuration * opacityDiff, 0.0f, ci::EaseNone());
	}
}

void MediaInterface::animateOff() {
	// TODO: settings
	tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this] { hide(); });
}

void MediaInterface::onSizeChanged() { layout(); }

}  // namespace ui
}  // namespace ds
