#include "smart_layout.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/util/string_util.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/image.h>


namespace ds {
namespace ui {

SmartLayout::SmartLayout(ds::ui::SpriteEngine& engine, const std::string& xmlLayoutFile,
						 const std::string xmlFileLocation)
	: ds::ui::LayoutSprite(engine)
	, mLayoutFile(xmlFileLocation + xmlLayoutFile)
	, mEventClient(engine.getNotifier(), [this](const ds::Event* m) {
		if (m) this->onAppEvent(*m);
	}) {

	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand(mLayoutFile), mSpriteMap, nullptr, "");

	runLayout();
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
	return setSpriteText(spriteName, ds::wstr_from_utf8(theText));
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::wstring& theText) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);
	ds::ui::MultilineText* sprM = getSprite<ds::ui::MultilineText>(spriteName);

	if (spr) {
		spr->setText(theText);
		runLayout();
	} else if (sprM) {
		sprM->setText(theText);
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Text for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteFont(const std::string& spriteName, const std::string& textCfgName) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		mEngine.getEngineCfg().getText(textCfgName).configure(*spr);
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Font " << textCfgName << " for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, const std::string& imagePath) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageFile(ds::Environment::expand(imagePath));
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, ds::Resource imageResource) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageResource(imageResource);
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteTapFn(const std::string& spriteName,
								 const std::function<void(ds::ui::Sprite*, const ci::Vec3f&)>& tapCallback) {
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
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to add child to " << spriteName);
	}
}


}  // namespace ui
}  // namespace ds
