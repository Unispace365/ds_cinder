#include "web_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/web.h>

#include "ds/ui/media/interface/web_interface.h"
#include "ds/ui/media/media_interface_builder.h"

namespace ds {
namespace ui {

WebPlayer::WebPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
	: ds::ui::Sprite(eng)
	, mWeb(nullptr)
	, mWebInterface(nullptr)
	, mEmbedInterface(embedInterface)
	, mShowInterfaceAtStart(true)
	, mKeyboardKeyScale(1.0f)
	, mKeyboardAllow(true)
	, mKeyboardAbove(true)
	, mAllowTouchToggle(true)
{
	mLayoutFixedAspect = true;
	enable(false);
	setTransparent(false);
	setColor(ci::Color::black());
}

void WebPlayer::setWebViewSize(const ci::vec2 webSize){
	mWebSize = webSize;
	if(mWeb){
		mWeb->setSize(mWebSize.x, mWebSize.y);
	}
}

void WebPlayer::setKeyboardParams(const float keyboardKeyScale, const bool keyboardAllow, const bool keyboardAbove){
	mKeyboardKeyScale = keyboardKeyScale;
	mKeyboardAllow = keyboardAllow;
	mKeyboardAbove = keyboardAbove;
	if(mWebInterface){
		mWebInterface->setKeyboardKeyScale(keyboardKeyScale);
		mWebInterface->setKeyboardAllow(keyboardAllow);
		mWebInterface->setKeyboardAbove(mKeyboardAbove);
	}
}

void WebPlayer::setAllowTouchToggle(const bool allowTouchToggle){
	mAllowTouchToggle = allowTouchToggle;
	if(mWebInterface){
		mWebInterface->setAllowTouchToggle(mAllowTouchToggle);
	}
}

void WebPlayer::setMedia(const std::string mediaPath){
	static const float fractionalWidthForContent = 0.6f;

	if(mWeb){
		mWeb->release();
		mWeb = nullptr;

		if(mWebInterface){
			mWebInterface->linkWeb(nullptr);
		}
	}

	mWeb = new ds::ui::Web(mEngine);
	mWeb->setDragScrolling(true);
	mWeb->setDragScrollingMinimumFingers(1);
	mWeb->setDrawWhileLoading(true);

	mWeb->setAddressChangedFn([this](const std::string& addy){
		if(mWebInterface){
			mWebInterface->updateWidgets();
		}
	});

	float targetW = mWebSize.x;
	float targetH = mWebSize.y;

	if((targetW == 0.0f) || (targetH == 0.0f)){
		targetW = mEngine.getWorldWidth() * fractionalWidthForContent;
		targetH = mEngine.getWorldHeight();
	}

	setWebViewSize(ci::vec2(targetW, targetH));

	addChildPtr(mWeb);
	mWeb->setUrl(mediaPath);
	mWeb->enable(false);

	if(mWebInterface){
		mWebInterface->release();
		mWebInterface = nullptr;
	}
	if(mEmbedInterface){
		mWebInterface = dynamic_cast<WebInterface*>(MediaInterfaceBuilder::buildMediaInterface(mEngine, this, this));

		if(mWebInterface){
			setKeyboardParams(mKeyboardKeyScale, mKeyboardAllow, mKeyboardAbove);
			setAllowTouchToggle(mAllowTouchToggle);
			mWebInterface->sendToFront();
		}
	}


	if(mWebInterface){
		if(mShowInterfaceAtStart){
			mWebInterface->show();
		} else {
			mWebInterface->setOpacity(0.0f);
			mWebInterface->hide();
		}
	}

	setSize(mWeb->getWidth(), mWeb->getHeight());
}

void WebPlayer::onSizeChanged(){
	layout();
}

void WebPlayer::layout(){
	if(mWeb){
		float scale = this->getHeight() / mWeb->getHeight();
		mWeb->setScale(scale);
	}

	if(mWebInterface && mEmbedInterface){
		mWebInterface->setSize(getWidth() * 2.0f / 3.0f, mWebInterface->getHeight());

		float yPos = getHeight() - mWebInterface->getHeight() - 50.0f;
		if(yPos < getHeight() / 2.0f) yPos = getHeight() / 2.0f;
		if(yPos + mWebInterface->getHeight() > getHeight()) yPos = getHeight() - mWebInterface->getHeight();
		mWebInterface->setPosition(getWidth() / 2.0f - mWebInterface->getWidth() / 2.0f, yPos);
	}
}

void WebPlayer::userInputReceived() {
	ds::ui::Sprite::userInputReceived();
	showInterface();
}

void WebPlayer::showInterface() {
	if(mWebInterface){
		mWebInterface->animateOn();
	}
}

void WebPlayer::hideInterface(){
	if(mWebInterface){
		mWebInterface->startIdling();
	}
}

void WebPlayer::setShowInterfaceAtStart(bool showInterfaceAtStart){
	mShowInterfaceAtStart = showInterfaceAtStart;
}

void WebPlayer::sendClick(const ci::vec3& globalClickPos){
	if(mWeb){
		mWeb->sendMouseClick(globalClickPos);
	}
}

ds::ui::Web* WebPlayer::getWeb(){
	return mWeb;
}


ds::ui::WebInterface* WebPlayer::getWebInterface(){
	return mWebInterface;
}

} // namespace ui
} // namespace ds
