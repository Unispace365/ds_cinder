#include "web_interface.h"

#include <ds/app/environment.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/web.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/text.h>

namespace ds {
namespace ui {

WebInterface::WebInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& sizey, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor)
	: MediaInterface(eng, sizey, backgroundColor)
	, mLinkedWeb(nullptr)
	, mBackButton(nullptr)
	, mForwardButton(nullptr)
	, mRefreshButton(nullptr)
	, mTouchToggle(nullptr)
{
	mForwardButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/next.png", "%APP%/data/images/media_interface/next.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mForwardButton);
	mForwardButton->setClickFn([this](){
		if(mLinkedWeb){
			mLinkedWeb->goForward();
			updateWidgets();
		}
	});

	mForwardButton->getNormalImage().setColor(buttonColor);
	mForwardButton->getHighImage().setColor(buttonColor / 2.0f);
	mForwardButton->setScale(sizey.y / mForwardButton->getHeight());

	mBackButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/prev.png", "%APP%/data/images/media_interface/prev.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mBackButton);
	mBackButton->setClickFn([this](){
		if(mLinkedWeb){
			mLinkedWeb->goBack();
			updateWidgets();
		}
	});

	mBackButton->getNormalImage().setColor(buttonColor);
	mBackButton->getHighImage().setColor(buttonColor / 2.0f);
	mBackButton->setScale(sizey.y / mBackButton->getHeight());


	mRefreshButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/refresh.png", "%APP%/data/images/media_interface/refresh.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mRefreshButton);
	mRefreshButton->setClickFn([this](){
		if(mLinkedWeb){
			mLinkedWeb->reload();
			updateWidgets();
		}
	});

	mRefreshButton->getNormalImage().setColor(buttonColor);
	mRefreshButton->getHighImage().setColor(buttonColor / 2.0f);
	mRefreshButton->setScale(sizey.y / mRefreshButton->getHeight());


	mTouchToggle = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/touch_unlocked.png", "%APP%/data/images/media_interface/touch_unlocked.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mTouchToggle);
	mTouchToggle->setClickFn([this](){
		if(mLinkedWeb){
			if(mLinkedWeb->isEnabled()){
				mLinkedWeb->enable(false);
			} else {
				mLinkedWeb->enable(true);
			}
			updateWidgets();
		}
	});

	mTouchToggle->getNormalImage().setColor(buttonColor);
	mTouchToggle->getHighImage().setColor(buttonColor / 2.0f);
	mTouchToggle->setScale(sizey.y / mTouchToggle->getHeight());


	updateWidgets();
}

void WebInterface::linkWeb(ds::ui::Web* linkedWeb){
	mLinkedWeb = linkedWeb;
	updateWidgets();
}


// Layout is called when the size is changed, so don't change the size in the layout
void WebInterface::onLayout(){
	if(mBackButton && mForwardButton && mRefreshButton && mTouchToggle){
		const float w = getWidth();
		const float h = getHeight();
		const float padding = h / 4.0f;

		float componentsWidth = (
			mBackButton->getScaleWidth() + padding +
			mForwardButton->getScaleWidth() + padding +
			mRefreshButton->getScaleWidth() + padding +
			mTouchToggle->getScaleWidth()
			);

		float margin = ((w - componentsWidth) * 0.5f);
		float xp = margin;

		mBackButton->setPosition(xp, (h * 0.5f) - mBackButton->getScaleHeight() / 2.0f);
		xp += mBackButton->getScaleWidth() + padding;

		mForwardButton->setPosition(xp, (h * 0.5f) - mForwardButton->getScaleHeight() / 2.0f);
		xp += mForwardButton->getScaleWidth() + padding;

		mRefreshButton->setPosition(xp, (h * 0.5f) - mRefreshButton->getScaleHeight() / 2.0f);
		xp += mRefreshButton->getScaleWidth() + padding;

		mTouchToggle->setPosition(xp, (h * 0.5f) - mTouchToggle->getScaleHeight() / 2.0f);
		xp += mTouchToggle->getScaleWidth() + padding;


	}
}

void WebInterface::updateWidgets(){
	if(mLinkedWeb && mForwardButton && mBackButton && mTouchToggle){
		// TODO: settings / config for disabled opacity / color
		if(mLinkedWeb->canGoBack()){
			mBackButton->enable(true);
			mBackButton->setOpacity(1.0f);
		} else {
			mBackButton->enable(false);
			mBackButton->setOpacity(0.25f);
		}
		if(mLinkedWeb->canGoForward()){
			mForwardButton->enable(true);
			mForwardButton->setOpacity(1.0f);
		} else {
			mForwardButton->enable(false);
			mForwardButton->setOpacity(0.25f);
		}

		if(mLinkedWeb->isEnabled()){
			mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
		} else {
			mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
		}
	}
	layout();
}

} // namespace ui
} // namespace ds
