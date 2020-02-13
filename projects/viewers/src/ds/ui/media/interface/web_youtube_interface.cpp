#include "stdafx.h"

#include "web_youtube_interface.h"

#include <ds/app/environment.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/web.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/text.h>

#include <ds/ui/layout/layout_sprite.h>

namespace ds {
namespace ui {

WebYoutubeInterface::WebYoutubeInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor)
	: MediaInterface(eng, sizey, backgroundColor)
	, mLinkedWeb(nullptr)
	, mBackTimeButton(nullptr)
	, mBackPageButton(nullptr)
	, mForwardTimeButton(nullptr)
	, mForwardPageButton(nullptr)
	, mTouchToggle(nullptr)
	, mAbleToTouchToggle(true)
	, mWebLocked(false)
	, mButtonHolder(nullptr)
	, mPlayButton(nullptr)
	, mIsFirstStart(true)
{
	enable(true);
	mButtonHolder = new ds::ui::SmartLayout(mEngine, "web_youtube_interface_layout.xml");
	if(mButtonHolder) {

		addChildPtr(mButtonHolder);
		mButtonHolder->setSize(sizey);

		if(mBackTimeButton = mButtonHolder->getSprite<ds::ui::ImageButton>("backTo_button")) {
			mBackTimeButton->setClickFn([this]() {
				if(mLinkedWeb) {
					//mLinkedWeb->goBack();
					auto code = ci::app::KeyEvent::KEY_LEFT;
					ci::app::KeyEvent event(mEngine.getWindow(), code, code, '<', 0, code);
					mLinkedWeb->sendKeyDownEvent(event);
					mLinkedWeb->sendKeyUpEvent(event);
					updateWidgets();
				}
			});
		}

		if(mPlayButton = mButtonHolder->getSprite<ds::ui::ImageButton>("play_button")) {
			mPlayButton->setClickFn([this]() {
				if(mLinkedWeb) {
					//mLinkedWeb->en();

					if(!mIsFirstStart) {
						auto code = ci::app::KeyEvent::KEY_SPACE;
						ci::app::KeyEvent event(mEngine.getWindow(), code, code, ' ', 0, code);
						mLinkedWeb->sendKeyDownEvent(event, false);
						mLinkedWeb->sendKeyUpEvent(event);
					} else {
						mLinkedWeb->setAllowClicks(true);
						mLinkedWeb->sendMouseClick(localToGlobal(mPlayButton->getPosition()));
						mLinkedWeb->setAllowClicks(false);
						mIsFirstStart = false;
					}


					updateWidgets();
				}
			});
		}

		if(mForwardTimeButton = mButtonHolder->getSprite<ds::ui::ImageButton>("nextTo_button")) {
			mForwardTimeButton->setClickFn([this]() {
				if(mLinkedWeb) {
					//mLinkedWeb->goForward();

					auto code = ci::app::KeyEvent::KEY_RIGHT;
					ci::app::KeyEvent event(mEngine.getWindow(), code, code, '>', 0, code);
					mLinkedWeb->sendKeyDownEvent(event);
					mLinkedWeb->sendKeyUpEvent(event);
					updateWidgets();
				}
			});
		}

		if(mForwardPageButton = mButtonHolder->getSprite<ds::ui::ImageButton>("next_button")) {
			mForwardPageButton->setClickFn([this]() {
				if(mLinkedWeb) {
					mLinkedWeb->goForward();
					updateWidgets();
				}
			});
		}

		if(mBackPageButton = mButtonHolder->getSprite<ds::ui::ImageButton>("back_button")) {
			mBackPageButton->setClickFn([this]() {
				if(mLinkedWeb) {
					mLinkedWeb->goBack();
					updateWidgets();
				}
			});
		}

		if(mTouchToggle = mButtonHolder->getSprite<ds::ui::ImageButton>("lock_button")) {
			mTouchToggle->setClickFn([this]() {
				if(mLinkedWeb) {
					if(mLinkedWeb->isEnabled()) {
						mLinkedWeb->enable(false);
					} else {
						mLinkedWeb->enable(true);
					}
					updateWidgets();
				}
			});
		}
	}



	if(!mAbleToTouchToggle) {
		mTouchToggle->hide();
	}

	const float padding = sizey.y / 4.0f;

	mMinWidth = (
		padding * 4.0f +
		mBackTimeButton->getScaleWidth() + padding +
		mForwardTimeButton->getScaleWidth() + padding +
		mBackPageButton->getScaleWidth() + padding +
		mForwardPageButton->getScaleWidth() + padding +
		mPlayButton->getScaleWidth() + padding +
		mTouchToggle->getScaleWidth() + padding * 2.0f
		);
	mMaxWidth = mMinWidth;

	updateWidgets();
}

void WebYoutubeInterface::setAllowTouchToggle(const bool allowTouchToggling) {
	mAbleToTouchToggle = allowTouchToggling;
	if(mTouchToggle) {
		if(mAbleToTouchToggle) {
			mTouchToggle->show();
		} else {
			mTouchToggle->hide();
		}

		layout();
	}
}

void WebYoutubeInterface::playYutube() {
	if(mLinkedWeb && mPlayButton) {
		mLinkedWeb->setAllowClicks(true);
		mLinkedWeb->sendMouseClick(localToGlobal(mPlayButton->getPosition()));
		mLinkedWeb->setAllowClicks(false);
		mIsFirstStart = false;
	}
}

void WebYoutubeInterface::animateOff() {
	mIdling = false;

	tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this] {
		hide();
	});
}

void WebYoutubeInterface::linkWeb(ds::ui::Web* linkedWeb) {
	if(mLinkedWeb) {
		mLinkedWeb->setAuthCallback(nullptr);
		mLinkedWeb->setLoadingUpdatedCallback(nullptr);
	}

	mLinkedWeb = linkedWeb;

	mLinkedWeb->setAllowClicks(false);

	updateWidgets();
}

// TODO: make this into a layoutsprite
// Layout is called when the size is changed, so don't change the size in the layout
void WebYoutubeInterface::onLayout() {
	const float w = getWidth();
	const float h = getHeight();
	if(mBackTimeButton && mForwardTimeButton && mTouchToggle && mPlayButton && mForwardPageButton && mBackPageButton) {
		const float padding = h / 4.0f;

		float componentsWidth = (
			padding * 4.0f +
			mBackTimeButton->getScaleWidth() + padding +
			mForwardTimeButton->getScaleWidth() + padding +
			mForwardPageButton->getScaleWidth() + padding +
			mBackPageButton->getScaleWidth() + padding +
			mPlayButton->getScaleWidth() + padding +
			mTouchToggle->getScaleWidth() + padding * 2.0f
			);

		if(!mAbleToTouchToggle) {
			componentsWidth -= (mTouchToggle->getScaleWidth() + padding);
		}

		float margin = ((w - componentsWidth) * 0.5f);
		float xp = margin;

		if(mButtonHolder)
		{
			mButtonHolder->setPosition(xp - padding, (h * 0.5f) - mButtonHolder->getScaleHeight() / 2.0f);
			mButtonHolder->runLayout();
		}
	}
}

void WebYoutubeInterface::updateWidgets() {
	if(mBackTimeButton && mForwardTimeButton && mTouchToggle && mPlayButton && mForwardPageButton && mBackPageButton && mLinkedWeb) {
		// TODO: settings / config for disabled opacity / color

		if(mLinkedWeb->canGoBack()) {
			mBackPageButton->enable(true);
			mBackPageButton->setOpacity(1.0f);
		} else {
			mBackPageButton->enable(false);
			mBackPageButton->setOpacity(0.25f);
		}
		if(mLinkedWeb->canGoForward()) {
			mForwardPageButton->enable(true);
			mForwardPageButton->setOpacity(1.0f);
		} else {
			mForwardPageButton->enable(false);
			mForwardPageButton->setOpacity(0.25f);
		}

		if(mLinkedWeb->isEnabled()) {
			if(!mWebLocked) {
				mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mWebLocked = true;
				mLinkedWeb->setAllowClicks(true);
			}
		} else {
			if(mWebLocked) {
				mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
				mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
				mWebLocked = false;
				mLinkedWeb->setAllowClicks(false);
			}
		}
	}

	layout();
}

} // namespace ui
} // namespace ds
