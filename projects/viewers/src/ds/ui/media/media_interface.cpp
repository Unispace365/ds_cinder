#include "stdafx.h"

#include "media_interface.h"


#include "glm/gtx/matrix_decompose.hpp"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#if defined(DS_NVPATH)
#include <ds/ui/sprite/svg_sprite.h>
#endif
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/button/layout_button.h>

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
	auto defaultHeight	 = mEngine.getViewersSettings().getFloat("ui:media_button:size", 0, 32.0f);
	mPlayHeight			= mEngine.getViewersSettings().getFloat("ui:media_button:play:size", 0, defaultHeight);
	mPauseHeight		 = mEngine.getViewersSettings().getFloat("ui:media_button:pause:size", 0, defaultHeight);
	mKeyboardHeight		 = mEngine.getViewersSettings().getFloat("ui:media_button:keyboard:size", 0, defaultHeight);
	mBackHeight			 = mEngine.getViewersSettings().getFloat("ui:media_button:back:size", 0, defaultHeight);
	mForwardHeight		 = mEngine.getViewersSettings().getFloat("ui:media_button:forward:size", 0, defaultHeight);
	mRefreshHeight		 = mEngine.getViewersSettings().getFloat("ui:media_button:refresh:size", 0, defaultHeight);
	mLockHeight			 = mEngine.getViewersSettings().getFloat("ui:media_button:lock:size", 0, defaultHeight);
	mLoopHeight			 = mEngine.getViewersSettings().getFloat("ui:media_button:loop:size", 0, defaultHeight);
	mVolumeHeight		 = mEngine.getViewersSettings().getFloat("ui:media_button:volume:size", 0, defaultHeight);
	mThumbnailHeight	 = mEngine.getViewersSettings().getFloat("ui:media_button:thumbnail:size", 0, defaultHeight);
	mVolumeSliderHeight = mEngine.getViewersSettings().getFloat("ui:media_button:volume_slider:size", 0, defaultHeight);
	mScrubBarHeight		= mEngine.getViewersSettings().getFloat("ui:media_button:scrub_bar:size", 0, defaultHeight);   


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
	auto baseFolder	   = engine.getViewersSettings().getString("ui:media_button:folder", 0, "");
	auto basePostfix   = engine.getViewersSettings().getString("ui:media_button:postfix", 0, "");
	auto baseExtension = engine.getViewersSettings().getString("ui:media_button:extension", 0, ".png");

	auto iconFileName = engine.getViewersSettings().getString(iconId, 0, "");
	auto folder		  = engine.getViewersSettings().getAttribute(iconId, 0, "folder", baseFolder);
	auto postfix	  = engine.getViewersSettings().getAttribute(iconId, 0, "postfix", basePostfix);
	auto extension	  = engine.getViewersSettings().getAttribute(iconId, 0, "ext", baseExtension);

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

ds::ui::LayoutButton* MediaInterface::createButton(const ci::vec2& sizey,const std::string& iconIdNormal, const std::string& iconIdHigh) {
	return createButton(mEngine, sizey, iconIdNormal, iconIdHigh);	
}

ds::ui::LayoutButton* MediaInterface::createButton(ds::ui::SpriteEngine& engine,const ci::vec2& sizey,const std::string& iconIdNormal,const std::string& iconIdHigh) {
	auto button			= new ds::ui::LayoutButton(engine, sizey.x, sizey.y);

	//check for svgs
	// This forces a fallback if DS_NVPATH is not defined. 
	// This is defined in the NV_PATH's Property Pages that are loaded into
	// the viewers project conditionally with the environment variable DS_VIEWERS_USE_NVPATH != false.
	// if you have a project that doesn't want to include nv_path and uses viewers
	// then you need to define DS_VIEWERS_USE_NVPATH as "false" in the enviornment variables.
#if defined(DS_NVPATH) 
	ds::ui::Sprite* normal		= nullptr;
	ds::ui::Sprite* high		= nullptr;
	auto suffix		= std::string(".svg");
	bool isNormalSvg = false;
	auto normalPath = composeIconPath(engine,iconIdNormal);
	if (normalPath.length() >= 4 && std::equal(suffix.rbegin(), suffix.rend(), normalPath.rbegin())) {
		// if the normal path ends with .svg then use svg
		isNormalSvg = true;
	} else {
		isNormalSvg = false;
	}

	bool isHighSvg	  = false;
	
	auto highPath = composeIconPath(engine,iconIdHigh);
	if (highPath.length() >= 4 && std::equal(suffix.rbegin(), suffix.rend(), highPath.rbegin())) {
		isHighSvg = true;
	}
	
	if (isNormalSvg) {
		auto normalSvg = new ds::ui::SvgSprite(engine);
		normalSvg->setFile(normalPath);
		normal = normalSvg;
	} else {
	
		// fallback to image
		// if not svg then use image
		normal = new ds::ui::Image(engine, normalPath, ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
	}
	
	
	if (isHighSvg) {
		auto highSvg = new ds::ui::SvgSprite(engine);
		highSvg->setFile(highPath);
		high = highSvg;
	} else {

		// fallback to image
		// if not svg then use image
		high = new ds::ui::Image(engine, highPath, ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
	}
#else
	auto			normalPath = composeIconPath(engine, iconIdNormal);
	auto			highPath   = composeIconPath(engine, iconIdHigh);
	ds::ui::Sprite* normal = nullptr;
	ds::ui::Sprite* high   = nullptr;
	normal = new ds::ui::Image(engine, normalPath, ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
	high   = new ds::ui::Image(engine, highPath, ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
#endif
	
	

	auto normalAspect = normal->getWidth() / normal->getHeight();
	auto highAspect = high->getWidth() / high->getHeight();
	normal->setSize(sizey.y * normalAspect, sizey.y);
	high->setSize(sizey.y * highAspect, sizey.y);

	button->getNormalSprite().addChildPtr(normal);
	button->getHighSprite().addChildPtr(high);
	button->runLayout();
	return button;
}

void MediaInterface::setButtonColor(ds::ui::LayoutButton* button, const ci::Color& normalColor, const ci::Color& highColor) {
	if (button->getNormalSprite().getChildren().empty() || button->getHighSprite().getChildren().empty()) return;
	button->getNormalSprite().getChildren()[0]->setColor(normalColor);
	button->getHighSprite().getChildren()[0]->setColor(highColor);
}

} // namespace ds::ui
