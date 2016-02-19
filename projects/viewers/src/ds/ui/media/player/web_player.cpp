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
	, mWebViewWidth(0.0f)
	, mWebViewHeight(0.0f)
{
	enable(false);
	setTransparent(false);
	setColor(ci::Color::black());
}

void WebPlayer::setWebViewSize(float webViewWidth, float webViewHeight){
	mWebViewWidth = webViewWidth;
	mWebViewHeight = webViewHeight;
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

	float targetW = mWebViewWidth;
	float targetH = mWebViewHeight;

	if((targetW == 0.0f) || (targetH == 0.0f)){
		targetW = mEngine.getWorldWidth() * fractionalWidthForContent;
		targetH = mEngine.getWorldHeight();
	}

	mWeb->setSize(targetW, targetH);

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
			mWebInterface->sendToFront();
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
		mWebInterface->setPosition(getWidth() / 2.0f - mWebInterface->getWidth() / 2.0f, getHeight() - mWebInterface->getHeight() - 50.0f);
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

void WebPlayer::sendClick(const ci::Vec3f& globalClickPos){
	if(mWeb){
		mWeb->sendMouseClick(globalClickPos);
	}
}

ds::ui::Web* WebPlayer::getWeb(){
	return mWeb;
}

} // namespace ui
} // namespace ds
