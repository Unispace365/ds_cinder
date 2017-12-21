#include "stdafx.h"

#include "smart_layout.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

SmartLayout::SmartLayout(ds::ui::SpriteEngine& engine, const std::string& xmlLayoutFile,
						 const std::string xmlFileLocation)
	: ds::ui::LayoutSprite(engine)
	, mLayoutFile(xmlFileLocation + xmlLayoutFile)
	, mNeedsLayout(false)
	, mEventClient(engine.getNotifier(), [this](const ds::Event* m) {
		if (m) this->onAppEvent(*m);
	}) {

	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand(mLayoutFile), mSpriteMap, nullptr, "", true);

	// Auto clear mNeedsLayout if client app runs layout manually
	setLayoutUpdatedFunction([this] { mNeedsLayout = false; });
}

bool SmartLayout::hasSprite(const std::string& spriteName) {
	return mSpriteMap.find(spriteName) != mSpriteMap.end();
}

ds::ui::Sprite* SmartLayout::getSprite(const std::string& spriteName) {
	auto findy = mSpriteMap.find(spriteName);
	if (findy != mSpriteMap.end()) {
		return findy->second;
	}
	return nullptr;
}

void SmartLayout::onAppEvent(const ds::Event& in_e) {
	if (mEventCallbacks.empty()) return;

	auto callbackIt = mEventCallbacks.find(in_e.mWhat);
	if (callbackIt != end(mEventCallbacks)) {
		(callbackIt->second)(in_e);
	}
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::string& theText) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if(spr) {
		spr->setText(theText);
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to set Text for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::wstring& theText) {
	return setSpriteText(spriteName, ds::utf8_from_wstr(theText));
}

void SmartLayout::setSpriteFont(const std::string& spriteName, const std::string& textCfgName) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		mEngine.getEngineCfg().getText(textCfgName).configure(*spr);
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to set Font " << textCfgName << " for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, const std::string& imagePath) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageFile(ds::Environment::expand(imagePath));
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, ds::Resource imageResource, bool cache) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		if(cache){
			sprI->setImageResource(imageResource, ds::ui::Image::IMG_CACHE_F);
		}else{
			sprI->setImageResource(imageResource);
		}
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteTapFn(const std::string& spriteName,
								 const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback) {
	ds::ui::Sprite* spr = getSprite(spriteName);
	if (spr && tapCallback) {
		spr->enable(true);
		spr->setTapCallback(tapCallback);
	}
}

void SmartLayout::addSpriteChild(const std::string spriteName, ds::ui::Sprite* newChild) {
	ds::ui::Sprite* spr = getSprite(spriteName);
	if (spr && newChild) {
		spr->addChildPtr(newChild);
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to add child to " << spriteName);
	}
}

void SmartLayout::onUpdateServer(const ds::UpdateParams& p){
    if(mNeedsLayout) {
        runLayout();
        mNeedsLayout = false;
    }
}


}  // namespace ui
}  // namespace ds
