#include "stdafx.h"

#include "layout_sprite.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/text_pango.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

SmartLayout::SmartLayout(Globals& g, std::string xmlLayoutFile)
  : ds::ui::Sprite(g.mEngine)
  , mGlobals(g)
  , mLayoutFile(xmlLayoutFile)
  , mRootLayout(nullptr)
  , mEventClient(g.mEngine.getNotifier(), [this](const ds::Event* m) {
      if (m) this->onAppEvent(*m);
  }) {

    ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/" + mLayoutFile), mSpriteMap);
    mRootLayout = dynamic_cast<ds::ui::LayoutSprite*>(mSpriteMap["root"]);

    runLayout(true);
}

void SmartLayout::onAppEvent(const ds::Event& in_e) {
    if (mEventCallbacks.empty()) return;

    auto callbackIt = mEventCallbacks.find(in_e.mWhat);
    if (callbackIt != end(mEventCallbacks)) {
        (callbackIt->second)(in_e);
    }
}

void SmartLayout::setTextSprite(std::string spriteName, std::string value) {
    return setTextSprite(spriteName, ds::wstr_from_utf8(value));
}

void SmartLayout::setTextSprite(std::string spriteName, std::wstring value) {
    ds::ui::Text*          spr  = dynamic_cast<ds::ui::Text*>(mSpriteMap[spriteName]);
    ds::ui::MultilineText* sprM = dynamic_cast<ds::ui::MultilineText*>(mSpriteMap[spriteName]);

    if (spr) {
        spr->setText(value);
    } else if (sprM) {
        sprM->setText(value);
    } else {
        DS_LOG_WARNING("Failed to set Text for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::setSpriteFont(std::string spriteName, std::string value) {
    ds::ui::Text*          spr  = dynamic_cast<ds::ui::Text*>(mSpriteMap[spriteName]);
    ds::ui::MultilineText* sprM = dynamic_cast<ds::ui::MultilineText*>(mSpriteMap[spriteName]);

    if (spr) {
        mGlobals.mEngine.getEngineCfg().getText(value).configure(*spr);
    } else if (sprM) {
        mGlobals.mEngine.getEngineCfg().getText(value).configure(*sprM);
    } else {
        DS_LOG_WARNING("Failed to set Font " << value << " for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::setImageSprite(std::string spriteName, std::string value) {
    ds::ui::Image* sprI = dynamic_cast<ds::ui::Image*>(mSpriteMap[spriteName]);

    if (sprI) {
        sprI->setImageFile(ds::Environment::expand("%APP%/data/images/" + value));
    } else {
        DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::setImageSprite(std::string spriteName, ds::Resource value) {
    ds::ui::Image* sprI = dynamic_cast<ds::ui::Image*>(mSpriteMap[spriteName]);

    if (sprI) {
        sprI->setImageResource(value);
    } else {
        DS_LOG_WARNING("Failed to set Image for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::setVideoSprite(std::string spriteName, std::string value) {
    ds::ui::Video* sprI = dynamic_cast<ds::ui::Video*>(mSpriteMap[spriteName]);

    if (sprI) {
        sprI->loadVideo(ds::Environment::expand("%APP%/data/videos/" + value));
    } else {
        DS_LOG_WARNING("Failed to set Video for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::setVideoSprite(std::string spriteName, ds::Resource value) {
    ds::ui::Video* sprI = dynamic_cast<ds::ui::Video*>(mSpriteMap[spriteName]);

    if (sprI) {
        sprI->setResource(value);
    } else {
        DS_LOG_WARNING("Failed to set Video for Sprite: " << spriteName);
        return *this;
    }

    runLayout(true);
    return *this;
}

void SmartLayout::listen(int type, std::function<void(const ds::Event&)> callback) {
    mEventCallbacks[type] = callback;
    return *this;
}

void SmartLayout::setSpriteTapFn(std::string spriteName,
                                         const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback) {
    ds::ui::Sprite* spr = dynamic_cast<ds::ui::Sprite*>(mSpriteMap[spriteName]);
    if (spr && tapCallback) {
        spr->enable(true);
        spr->setTapCallback(tapCallback);
    }
    return *this;
}

void SmartLayout::addSpriteChild(std::string spriteName, ds::ui::Sprite* newChild) {
    ds::ui::Sprite* spr = dynamic_cast<ds::ui::Sprite*>(mSpriteMap[spriteName]);
    if (spr && newChild) {
        spr->addChildPtr(newChild);
    } else {
        DS_LOG_WARNING("Failed to add child to " << spriteName);
    }
    return *this;
}



} // namespace ui
} // namespace ds
