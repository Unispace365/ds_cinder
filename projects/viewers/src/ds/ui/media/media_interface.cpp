#include "stdafx.h"

#include "media_interface.h"


#include "glm/gtx/matrix_decompose.hpp"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/video.h>

#include "ds/ui/media/interface/video_scrub_bar.h"
#include "ds/ui/media/interface/video_volume_control.h"

namespace ds::ui {

float MediaInterface::mPlayHeight = 0.0f;
float MediaInterface::mPauseHeight = 0.0f;
float MediaInterface::mKeyboardHeight = 0.0f;
float MediaInterface::mBackHeight	  = 0.0f;
float MediaInterface::mForwardHeight  = 0.0f;
float MediaInterface::mRefreshHeight  = 0.0f;
float MediaInterface::mLockHeight	  = 0.0f;
float MediaInterface::mLoopHeight	  = 0.0f;
float MediaInterface::mVolumeHeight	  = 0.0f;
float MediaInterface::mThumbnailHeight = 0.0f;
float MediaInterface::mVolumeSliderHeight = 0.0f;
float MediaInterface::mScrubBarHeight	  = 0.0f;



MediaInterface::MediaInterface(ds::ui::SpriteEngine& eng, int type, const ci::vec2& sizey,
							   const ci::Color backgroundColor)
  : ds::ui::Sprite(eng, sizey.x, sizey.y)
  , mType(type)
  , mBackground(nullptr)
  , mAnimateDuration(0.35f)
  , mMinWidth(sizey.y)
  , mMaxWidth(sizey.x)
  , mIdling(false)
  , mCanIdle(true)
  , mCanDisplay(true)
  , mCanLock(false)
  , mInterfaceIdleSettings(5.0f) {

	// TODO: settings?
	auto defaultHeight	 = mEngine.getWafflesSettings().getFloat("ui:media_button:size", 0, 32.0f);
	 mPlayHeight			 = mEngine.getWafflesSettings().getFloat("ui:media_button:play:size", 0, defaultHeight);
	 mPauseHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:pause:size", 0, defaultHeight);
	 mKeyboardHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:keyboard:size", 0, defaultHeight);
	 mBackHeight			 = mEngine.getWafflesSettings().getFloat("ui:media_button:back:size", 0, defaultHeight);
	 mForwardHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:forward:size", 0, defaultHeight);
	 mRefreshHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:refresh:size", 0, defaultHeight);
	 mLockHeight			 = mEngine.getWafflesSettings().getFloat("ui:media_button:lock:size", 0, defaultHeight);
	 mLoopHeight			 = mEngine.getWafflesSettings().getFloat("ui:media_button:loop:size", 0, defaultHeight);
	 mVolumeHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:volume:size", 0, defaultHeight);
	 mThumbnailHeight = mEngine.getWafflesSettings().getFloat("ui:media_button:thumbnail:size", 0, defaultHeight);
	 mVolumeSliderHeight = mEngine.getWafflesSettings().getFloat("ui:media_button:volume_slider:size", 0, defaultHeight);
	 mScrubBarHeight		 = mEngine.getWafflesSettings().getFloat("ui:media_button:scrub_bar:size", 0, defaultHeight);   


	const float backOpacccy = 0.95f;

	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->setTransparent(false);
	mBackground->setColor(backgroundColor);
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

	if (visible()) {
		ci::vec3  scale, translation, skew;
		ci::vec4  persp;
		glm::quat orient;
		glm::decompose(this->getParent()->getGlobalTransform(), scale, orient, translation, skew, persp);
		setScale(1.f / scale.x, 1.f / scale.y);
	}
}

// Layout is called when the size is changed, so don't change the size in the layout
void MediaInterface::layout() {
	const float w  = getWidth();
	const float h  = getHeight();
	const float ww = mEngine.getWorldWidth();
	onLayout();
	if (mBackground) {
		// Ensure the maximum width always fits within the window
		mMaxWidth = glm::min(mMaxWidth, ww);
		// mMaxWidth  = glm::min(mMaxWidth, 900.f);
		float newW = glm::clamp(w, mMinWidth, mMaxWidth);


		mBackground->setSize(newW, h);
		mBackground->setCenter(0.5f, 0.0f);
		mBackground->setPosition(w / 2.0f, 0.0f);
	}
}

void MediaInterface::setBackgroundColorA(const ci::ColorA backgroundColor) {
	if (mBackground) mBackground->setColorA(backgroundColor);
}

void MediaInterface::setBackgroundColor(ci::ColorA newColor) {
	if (mBackground) mBackground->setColorA(newColor);
}

void MediaInterface::setBackgroundColor(ci::Color newColor) {
	if (mBackground) mBackground->setColor(newColor);
}

void MediaInterface::show() {
	if (mCanDisplay) {
		Sprite::show();
	}
}

void MediaInterface::animateOn() {
	if (!mCanDisplay) return;
	resetIdleTimer();
	show();

	float opacityDiff = (1.0f - getOpacity());
	if (opacityDiff > 0.0f) {
		tweenOpacity(1.0f, mAnimateDuration * opacityDiff, 0.0f, ci::EaseNone());
	}

	mEngine.getNotifier().notify(MediaInterfaceShownEvent(this));
}

void MediaInterface::animateOff() {
	// TODO: settings
	tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this] { hide(); });

	mEngine.getNotifier().notify(MediaInterfaceHiddenEvent(this));
}

std::string MediaInterface::composeIconPath(std::string iconId) {
	return composeIconPath(mEngine, iconId);
}

std::string MediaInterface::composeIconPath(ds::ui::SpriteEngine& engine, std::string iconId) {
	auto baseFolder	   = engine.getWafflesSettings().getString("ui:media_button:folder", 0, "");
	auto basePostfix   = engine.getWafflesSettings().getString("ui:media_button:postfix", 0, "");
	auto baseExtension = engine.getWafflesSettings().getString("ui:media_button:extension", 0, ".png");

	auto iconFileName = engine.getWafflesSettings().getString(iconId, 0, "");
	auto folder		  = engine.getWafflesSettings().getAttribute(iconId, 0, "folder", baseFolder);
	auto postfix	  = engine.getWafflesSettings().getAttribute(iconId, 0, "postfix", basePostfix);
	auto extension	  = engine.getWafflesSettings().getAttribute(iconId, 0, "ext", baseExtension);

	std::string path = folder + iconFileName + postfix + extension;
	path			 = ds::Environment::expand(path);

	return path;
}

void MediaInterface::onSizeChanged() {
	layout();

	ci::vec3  scale, translation, skew;
	ci::vec4  persp;
	glm::quat orient;
	glm::decompose(this->getParent()->getGlobalTransform(), scale, orient, translation, skew, persp);
	setScale(1.f / scale.x, 1.f / scale.y);
}

} // namespace ds::ui
