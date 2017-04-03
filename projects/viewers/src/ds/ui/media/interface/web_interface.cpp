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

#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/soft_keyboard/soft_keyboard_defs.h>
#include <ds/ui/soft_keyboard/soft_keyboard_button.h>
#include <ds/ui/soft_keyboard/soft_keyboard_builder.h>

namespace ds {
namespace ui {

WebInterface::WebInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor)
	: MediaInterface(eng, sizey, backgroundColor)
	, mLinkedWeb(nullptr)
	, mKeyboardArea(nullptr)
	, mKeyboard(nullptr)
	, mKeyboardShowing(false)
	, mKeyboardAbove(true)
	, mKeyboardButton(nullptr)
	, mBackButton(nullptr)
	, mForwardButton(nullptr)
	, mRefreshButton(nullptr)
	, mTouchToggle(nullptr)
	, mKeyboardKeyScale(1.0f)
	, mAbleToTouchToggle(true)
	, mKeyboardAllowed(true)
	, mKeyboardAutoDisablesTimeout(true)
	, mWebLocked(false)
{
	mKeyboardArea = new ds::ui::Sprite(mEngine, 10.0f, 10.0f);
	mKeyboardArea->setTransparent(false);
	mKeyboardArea->setColor(ci::Color(0.0f, 0.0f, 0.0f));
	mKeyboardArea->setCornerRadius(15.0f);
	mKeyboardArea->setOpacity(0.0f);
	mKeyboardArea->hide();
	addChildPtr(mKeyboardArea);

	mKeyboardButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/media_interface/keyboard.png", "%APP%/data/images/media_interface/keyboard.png", (sizey.y - buttonHeight) / 2.0f);
	addChildPtr(mKeyboardButton);
	mKeyboardButton->setClickFn([this](){
		mKeyboardShowing = !mKeyboardShowing;
		updateWidgets();
	});

	mKeyboardButton->getNormalImage().setColor(buttonColor);
	mKeyboardButton->getHighImage().setColor(buttonColor / 2.0f);
	mKeyboardButton->setScale(sizey.y / mKeyboardButton->getHeight());


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

	if(!mAbleToTouchToggle){
		mTouchToggle->hide();
	}



	const float padding = sizey.y / 4.0f;

	mMinWidth = (
		mKeyboardButton->getScaleWidth() + padding +
		mBackButton->getScaleWidth() + padding +
		mForwardButton->getScaleWidth() + padding +
		mRefreshButton->getScaleWidth() + padding +
		mTouchToggle->getScaleWidth() + padding * 2.0f
		);
	mMaxWidth = mMinWidth;

	updateWidgets();
}

void WebInterface::setKeyboardAllow(const bool keyboardAllowed){
	mKeyboardAllowed = keyboardAllowed;
	if(mKeyboardButton){
		if(mKeyboardAllowed){
			mKeyboardButton->show();
		} else {
			mKeyboardButton->hide();
		}
		layout();
	}
}


void WebInterface::setKeyboardAbove(const bool kerboardAbove){
	mKeyboardAbove = kerboardAbove;

	layout();
}

void WebInterface::setAllowTouchToggle(const bool allowTouchToggling){
	mAbleToTouchToggle = allowTouchToggling;
	if(mTouchToggle){
		if(mAbleToTouchToggle){
			mTouchToggle->show();
		} else {
			mTouchToggle->hide();
		}

		layout();
	}
}


void WebInterface::setKeyboardDisablesTimeout(const bool doAutoTimeout){
	mKeyboardAutoDisablesTimeout = doAutoTimeout;
}

void WebInterface::setKeyboardKeyScale(const float newKeyScale){
	mKeyboardKeyScale = newKeyScale;
}

void WebInterface::animateOff(){
	mIdling = false;
	tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::EaseNone(), [this]{
		hide();
		mKeyboardShowing = false;
		updateWidgets();
	});
}

void WebInterface::linkWeb(ds::ui::Web* linkedWeb){
	mLinkedWeb = linkedWeb;
	updateWidgets();
}

// TODO: make this into a layoutsprite
// Layout is called when the size is changed, so don't change the size in the layout
void WebInterface::onLayout(){
	const float w = getWidth();
	const float h = getHeight();
	if(mKeyboardButton && mBackButton && mForwardButton && mRefreshButton && mTouchToggle){
		const float padding = h / 4.0f;

		float componentsWidth = (
			mKeyboardButton->getScaleWidth() + padding +
			mBackButton->getScaleWidth() + padding +
			mForwardButton->getScaleWidth() + padding +
			mRefreshButton->getScaleWidth() + padding +
			mTouchToggle->getScaleWidth()
			);

		if(!mKeyboardAllowed){
			componentsWidth -= (mKeyboardButton->getScaleWidth() + padding);
		}

		if(!mAbleToTouchToggle){
			componentsWidth -= (mTouchToggle->getScaleWidth() + padding);
		}

		float margin = ((w - componentsWidth) * 0.5f);
		float xp = margin;

		if(mKeyboardAllowed){
			mKeyboardButton->setPosition(xp, (h * 0.5f) - mKeyboardButton->getScaleHeight() / 2.0f);
			xp += mKeyboardButton->getScaleWidth() + padding;
		}

		mBackButton->setPosition(xp, (h * 0.5f) - mBackButton->getScaleHeight() / 2.0f);
		xp += mBackButton->getScaleWidth() + padding;

		mForwardButton->setPosition(xp, (h * 0.5f) - mForwardButton->getScaleHeight() / 2.0f);
		xp += mForwardButton->getScaleWidth() + padding;

		mRefreshButton->setPosition(xp, (h * 0.5f) - mRefreshButton->getScaleHeight() / 2.0f);
		xp += mRefreshButton->getScaleWidth() + padding;

		if(mAbleToTouchToggle){
			mTouchToggle->setPosition(xp, (h * 0.5f) - mTouchToggle->getScaleHeight() / 2.0f);
			xp += mTouchToggle->getScaleWidth() + padding;
		}
	}

	if(mKeyboardArea){
		// center the keyboard area above this sprite
		const float keyboardW = mKeyboardArea->getWidth();
		const float keyboardH = mKeyboardArea->getHeight();
		float yp = h;
		if(mKeyboardAbove) yp = -keyboardH;

		mKeyboardArea->setPosition((w - keyboardW) * 0.5f, yp);
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
			if(!mWebLocked){
				mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_locked.png", ds::ui::Image::IMG_CACHE_F);
				mWebLocked = true;
			}
		} else {
			if(mWebLocked){
				mTouchToggle->getHighImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
				mTouchToggle->getNormalImage().setImageFile("%APP%/data/images/media_interface/touch_unlocked.png", ds::ui::Image::IMG_CACHE_F);
				mWebLocked = false;
			}
		}
	}

	if(mKeyboardArea){
		if(mKeyboardShowing){
			if(mKeyboardAutoDisablesTimeout){
				setCanTimeout(false);
			}
			if(!mKeyboard){
				ds::ui::SoftKeyboardSettings sks;
				sks.mKeyScale = mKeyboardKeyScale;
				mKeyboard = ds::ui::SoftKeyboardBuilder::buildExtendedKeyboard(mEngine, sks);
				mKeyboardArea->addChildPtr(mKeyboard);

				const float keyW = mKeyboard->getScaleWidth();
				const float keyH = mKeyboard->getScaleHeight();
				const float areaW = keyW + 30.0f;
				const float areaH = keyH + 30.0f;
				mKeyboardArea->setSize(areaW, areaH);
				mKeyboard->setPosition(
					(areaW - keyW) * 0.5f,
					(areaH - keyH) * 0.5f
				);

				mKeyboard->setKeyPressFunction([this](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
					if(mLinkedWeb){
						// spoof a keyevent to send to the web
						bool send = true;
						int code = 0;
			
						if(keyType == ds::ui::SoftKeyboardDefs::kShift){
							send = false;
						} else if(keyType == ds::ui::SoftKeyboardDefs::kDelete){
							code = ci::app::KeyEvent::KEY_BACKSPACE;
							send = false;
							ci::app::KeyEvent event(
								mEngine.getWindow(),
								code,
								code,
								'	',
								0,
								code
								);
							mLinkedWeb->sendKeyDownEvent(event);
							mLinkedWeb->sendKeyUpEvent(event);
						} else if(keyType == ds::ui::SoftKeyboardDefs::kEnter){
							code = ci::app::KeyEvent::KEY_RETURN;
							send = false;
							ci::app::KeyEvent event(
								mEngine.getWindow(),
								code,
								code,
								'\r',
								0,
								code
								);
							mLinkedWeb->sendKeyDownEvent(event);
							mLinkedWeb->sendKeyUpEvent(event);
						} else if(keyType == ds::ui::SoftKeyboardDefs::kTab){
							code = ci::app::KeyEvent::KEY_TAB;
							send = false;
							ci::app::KeyEvent event(
								mEngine.getWindow(),
								code,
								code,
								'	',
								0,
								code
								);
							mLinkedWeb->sendKeyDownEvent(event);
							mLinkedWeb->sendKeyUpEvent(event);
						}

						if(send){
							ci::app::KeyEvent event(
								mEngine.getWindow(),
								code,
								0,
								(char)character.c_str()[0],
								0,
								code
							);
							mLinkedWeb->sendKeyDownEvent(event);
							mLinkedWeb->sendKeyUpEvent(event);
						}
					}
				});
				layout();
			}

			if(!mKeyboardArea->visible()){
				mKeyboardArea->show();
				mKeyboardArea->tweenOpacity(0.98f, mAnimateDuration, 0.0f, ci::easeNone);
			}
		} else {
			if(mKeyboardAutoDisablesTimeout){
				setCanTimeout(true);
			}
			if(mKeyboardArea->visible()){
				mKeyboardArea->tweenOpacity(0.0f, mAnimateDuration, 0.0f, ci::easeNone, [this](){
					mKeyboardArea->hide();
				});
			}
		}
	}

	layout();
}

} // namespace ui
} // namespace ds
