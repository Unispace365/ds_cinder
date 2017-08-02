#include "stdafx.h"

#include "smart_layout.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

SmartLayout::SmartLayout(ds::ui::SpriteEngine& engine, std::string xmlLayoutFile)
  : ds::ui::LayoutSprite(engine)
  , mLayoutFile(xmlLayoutFile)
  , mEventClient(engine.getNotifier(), [this](const ds::Event* m) {
	  if (m) this->onAppEvent(*m);
  }) {

	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/" + mLayoutFile), mSpriteMap, nullptr, "", true);

	runLayout();
}

bool SmartLayout::hasSprite(const std::string& spriteName){
	return mSpriteMap.find(spriteName) != mSpriteMap.end();
}

ds::ui::Sprite* SmartLayout::getSprite(const std::string& spriteName){
	auto findy = mSpriteMap.find(spriteName);
	if(findy != mSpriteMap.end()){
		return findy->second;
	}
	return nullptr;
}

template <typename T>
T* ds::ui::SmartLayout::getSprite(const std::string& spriteName){
	return dynamic_cast<T*>(getSprite(spriteName));
}

void SmartLayout::onAppEvent(const ds::Event& in_e) {
	if (mEventCallbacks.empty()) return;

	auto callbackIt = mEventCallbacks.find(in_e.mWhat);
	if (callbackIt != end(mEventCallbacks)) {
		(callbackIt->second)(in_e);
	}
}

void SmartLayout::listenToEvents(size_t type, std::function<void(const ds::Event&)> callback) {
	mEventCallbacks[type] = callback;
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::string& value) {
	return setSpriteText(spriteName, ds::wstr_from_utf8(value));
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::wstring& value) {
	ds::ui::Text* spr  = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		spr->setText(value);
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Text for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteFont(const std::string& spriteName, const std::string& value) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		mEngine.getEngineCfg().getText(value).configure(*spr);
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Font " << value << " for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, const std::string& value) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageFile(ds::Environment::expand("%APP%/data/images/" + value));
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, ds::Resource value) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageResource(value);
		runLayout();
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
		runLayout();
	} else {
		DS_LOG_WARNING("Failed to add child to " << spriteName);
	}
}


}  // namespace ui
}  // namespace ds
