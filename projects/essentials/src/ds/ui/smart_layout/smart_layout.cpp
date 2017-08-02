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

void SmartLayout::onAppEvent(const ds::Event& in_e) {
	if (mEventCallbacks.empty()) return;

	auto callbackIt = mEventCallbacks.find(in_e.mWhat);
	if (callbackIt != end(mEventCallbacks)) {
		(callbackIt->second)(in_e);
	}
}

void SmartLayout::listen(size_t type, std::function<void(const ds::Event&)> callback) {
	mEventCallbacks[type] = callback;
}

void SmartLayout::setSpriteText(std::string spriteName, std::string value) {
	return setSpriteText(spriteName, ds::wstr_from_utf8(value));
}

void SmartLayout::setSpriteText(std::string spriteName, std::wstring value) {
	ds::ui::Text*		   spr  = dynamic_cast<ds::ui::Text*>(mSpriteMap[spriteName]);

	if (spr) {
		spr->setText(value);
	} else {
		DS_LOG_WARNING("Failed to set Text for Sprite: " << spriteName);
	}

	runLayout();
}

void SmartLayout::setSpriteFont(std::string spriteName, std::string value) {
	ds::ui::Text*		   spr  = dynamic_cast<ds::ui::Text*>(mSpriteMap[spriteName]);

	if (spr) {
		mEngine.getEngineCfg().getText(value).configure(*spr);
	} else {
		DS_LOG_WARNING("Failed to set Font " << value << " for Sprite: " << spriteName);
	}

	runLayout();
}

void SmartLayout::setSpriteImage(std::string spriteName, std::string value) {
	ds::ui::Image* sprI = dynamic_cast<ds::ui::Image*>(mSpriteMap[spriteName]);

	if (sprI) {
		sprI->setImageFile(ds::Environment::expand("%APP%/data/images/" + value));
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}

	runLayout();
}

void SmartLayout::setSpriteImage(std::string spriteName, ds::Resource value) {
	ds::ui::Image* sprI = dynamic_cast<ds::ui::Image*>(mSpriteMap[spriteName]);

	if (sprI) {
		sprI->setImageResource(value);
	} else {
		DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
	}

	runLayout();
}

void SmartLayout::setSpriteTapFn(std::string spriteName,
								 const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback) {
	ds::ui::Sprite* spr = dynamic_cast<ds::ui::Sprite*>(mSpriteMap[spriteName]);
	if (spr && tapCallback) {
		spr->enable(true);
		spr->setTapCallback(tapCallback);
	}
}

void SmartLayout::addSpriteChild(std::string spriteName, ds::ui::Sprite* newChild) {
	ds::ui::Sprite* spr = dynamic_cast<ds::ui::Sprite*>(mSpriteMap[spriteName]);
	if (spr && newChild) {
		spr->addChildPtr(newChild);
	} else {
		DS_LOG_WARNING("Failed to add child to " << spriteName);
	}
}


}  // namespace ui
}  // namespace ds
