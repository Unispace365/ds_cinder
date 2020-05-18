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
	, mPlayButton(nullptr)
	, mIsFirstStart(true) {


	enable(true);


	mBackPageButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/prev.png", "%APP%/data/images/media_interface/prev.png", (sizey.y - buttonHeight) / 2.0f);
	setupButton(mBackPageButton, buttonColor, sizey.y);
	mBackPageButton->setClickFn([this]() {
		if (mLinkedWeb) {
			mLinkedWeb->goBack();
			updateWidgets();
		}
	});



	mBackTimeButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/rewind.png", "%APP%/data/images/media_interface/rewind.png", (sizey.y - buttonHeight) / 2.0f);	
	setupButton(mBackTimeButton, buttonColor, sizey.y);
	mBackTimeButton->setClickFn([this]() {
		if (mLinkedWeb) {
			//mLinkedWeb->goBack();
			auto code = ci::app::KeyEvent::KEY_j;
			ci::app::KeyEvent event(mEngine.getWindow(), code, code, 'j', 0, code);
			mLinkedWeb->sendKeyDownEvent(event);
			mLinkedWeb->sendKeyUpEvent(event);
			updateWidgets();
		}
	});

	
	mPlayButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/play_pause.png", "%APP%/data/images/media_interface/play_pause.png", (sizey.y - buttonHeight) / 2.0f);
	setupButton(mPlayButton, buttonColor, sizey.y);
	mPlayButton->setClickFn([this]() {
		if (mLinkedWeb) {
			if (!mIsFirstStart) {
				auto code = ci::app::KeyEvent::KEY_k;
				ci::app::KeyEvent event(mEngine.getWindow(), code, code, 'k', 0, code);
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


	mForwardTimeButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/fast_forward.png", "%APP%/data/images/media_interface/fast_forward.png", (sizey.y - buttonHeight) / 2.0f);
	setupButton(mForwardTimeButton, buttonColor, sizey.y);
	mForwardTimeButton->setClickFn([this]() {
		if (mLinkedWeb) {
			auto code = ci::app::KeyEvent::KEY_l;
			ci::app::KeyEvent event(mEngine.getWindow(), code, code, 'l', 0, code);
			mLinkedWeb->sendKeyDownEvent(event);
			mLinkedWeb->sendKeyUpEvent(event);
			updateWidgets();
		}
	});

	mForwardPageButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/next.png", "%APP%/data/images/media_interface/next.png", (sizey.y - buttonHeight) / 2.0f);
	setupButton(mForwardPageButton, buttonColor, sizey.y);
	mForwardPageButton->setClickFn([this]() {
		if (mLinkedWeb) {
			mLinkedWeb->goForward();
			updateWidgets();
		}
	});
		

	mTouchToggle = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/touch_unlocked.png", "%APP%/data/images/media_interface/touch_unlocked.png", (sizey.y - buttonHeight) / 2.0f);
	setupButton(mTouchToggle, buttonColor, sizey.y);
	mTouchToggle->setClickFn([this]() {
		if (mLinkedWeb) {
			if (mLinkedWeb->isEnabled()) {
				mLinkedWeb->enable(false);
			} else {
				mLinkedWeb->enable(true);
			}
			updateWidgets();
		}
	});



	if (!mAbleToTouchToggle) {
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


void WebYoutubeInterface::setupButton(ds::ui::ImageButton* bs, ci::Color buttonColor, float height) {
	bs->getNormalImage().setColor(buttonColor);
	bs->getHighImage().setColor(buttonColor / 2.0f);
	bs->setScale(height / bs->getHeight());
	addChildPtr(bs);
}


void WebYoutubeInterface::setAllowTouchToggle(const bool allowTouchToggling) {
	mAbleToTouchToggle = allowTouchToggling;
	if (mTouchToggle) {
		if (mAbleToTouchToggle) {
			mTouchToggle->show();
		} else {
			mTouchToggle->hide();
		}

		layout();
	}
}

void WebYoutubeInterface::playYutube() {
	if (mLinkedWeb && mPlayButton) {
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
	if (mLinkedWeb) {
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
	if (mBackTimeButton && mForwardTimeButton && mTouchToggle && mPlayButton && mForwardPageButton && mBackPageButton) {
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

		if (!mAbleToTouchToggle) {
			componentsWidth -= (mTouchToggle->getScaleWidth() + padding);
		}

		float margin = ((w - componentsWidth) * 0.5f);
		float xp = margin;


		mBackPageButton->setPosition(xp, (h * 0.5f) - mBackPageButton->getScaleHeight() / 2.0f);
		xp += mBackPageButton->getScaleWidth() + padding;

		mBackTimeButton->setPosition(xp, (h * 0.5f) - mBackTimeButton->getScaleHeight() / 2.0f);
		xp += mBackTimeButton->getScaleWidth() + padding;

		mPlayButton->setPosition(xp, (h * 0.5f) - mPlayButton->getScaleHeight() / 2.0f);
		xp += mPlayButton->getScaleWidth() + padding;

		mForwardTimeButton->setPosition(xp, (h * 0.5f) - mForwardTimeButton->getScaleHeight() / 2.0f);
		xp += mForwardTimeButton->getScaleWidth() + padding;

		mForwardPageButton->setPosition(xp, (h * 0.5f) - mForwardPageButton->getScaleHeight() / 2.0f);
		xp += mForwardPageButton->getScaleWidth() + padding;

		if (mAbleToTouchToggle) {
			mTouchToggle->setPosition(xp, (h * 0.5f) - mTouchToggle->getScaleHeight() / 2.0f);
			xp += mTouchToggle->getScaleWidth() + padding;
		}
	}
}

void WebYoutubeInterface::updateWidgets() {
	if (mBackTimeButton && mForwardTimeButton && mTouchToggle && mPlayButton && mForwardPageButton && mBackPageButton && mLinkedWeb) {
		// TODO: settings / config for disabled opacity / color

		if (mLinkedWeb->canGoBack()) {
			mBackPageButton->enable(true);
			mBackPageButton->setOpacity(1.0f);
		} else {
			mBackPageButton->enable(false);
			mBackPageButton->setOpacity(0.25f);
		}
		if (mLinkedWeb->canGoForward()) {
			mForwardPageButton->enable(true);
			mForwardPageButton->setOpacity(1.0f);
		} else {
			mForwardPageButton->enable(false);
			mForwardPageButton->setOpacity(0.25f);
		}

		if (mLinkedWeb->isEnabled()) {
			if (!mWebLocked) {
				mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mWebLocked = true;
				mLinkedWeb->setAllowClicks(true);
			}
		} else {
			if (mWebLocked) {
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
