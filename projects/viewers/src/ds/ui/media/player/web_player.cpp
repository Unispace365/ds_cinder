#include "web_player.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <ds/ui/sprite/web.h>

#include "ds/ui/media/interface/web_interface.h"

namespace ds {
namespace ui {

WebPlayer::WebPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
	: ds::ui::Sprite(eng)
	, mWeb(nullptr)
	, mWebInterface(nullptr)
	, mEmbedInterface(embedInterface)
{
	enable(false);
	setTransparent(false);
	setColor(ci::Color::black());
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

	mWeb->setSize(mEngine.getWorldWidth() * fractionalWidthForContent, mEngine.getWorldHeight());

	addChildPtr(mWeb);
	mWeb->setUrl(mediaPath);
	mWeb->enable(false);

	if(!mWebInterface){
		mWebInterface = new WebInterface(mEngine, ci::Vec2f(400.0f, 50.0f), 25.0f, ci::Color::white(), ci::Color::black());
	}

	if(mEmbedInterface){
		addChildPtr(mWebInterface);
		mWebInterface->sendToFront();
	}

	if(mWebInterface && mWeb){
		mWebInterface->linkWeb(mWeb);
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
		mWebInterface->setPosition(getWidth() / 2.0f - mWebInterface->getWidth() / 2.0f, getHeight() - mWebInterface->getHeight() - 50.0f);
	}
}

MediaInterface* WebPlayer::getExternalInterface() {
	return (mEmbedInterface ? nullptr : mWebInterface);
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

void WebPlayer::sendClick(const ci::Vec3f& globalClickPos){
	if(mWeb){
		mWeb->sendMouseClick(globalClickPos);
	}
}

} // namespace ui
} // namespace ds
