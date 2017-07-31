#pragma once
#ifndef DS_UI_LAYOUT_LAYOUT_SPRITE
#define DS_UI_LAYOUT_LAYOUT_SPRITE


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::SmartLayout
*        SmartLayout combines a layout sprite with XML importing.
*/
class SmartLayout : public ds::ui::LayoutSprite  {
public:
    SmartLayout(ds::ui::SpriteEngine& engine, std::string xmlLayoutFile);

    bool hasSprite(std::string spriteName) { return mSpriteMap[spriteName] != nullptr; }

    ds::ui::Sprite* getSprite(std::string spriteName) { return mSpriteMap.at(spriteName); }
    template <typename T>
    T* getSprite(std::string spriteName) { return dynamic_cast<T*>(mSpriteMap.at(spriteName);); }

    void addSpriteChild(std::string spriteName, ds::ui::Sprite* newChild);

    // Text sprite functions
    void setSpriteText(std::string, std::wstring);
    void setSpriteText(std::string, std::string);
    void setSpriteFont(std::string, std::string);

    // Image sprite functions
    void setSpriteImage(std::string, std::string);
    void setSpriteImage(std::string, ds::Resource);

    void listen(size_t type, std::function<void(const ds::Event&)> callback);
    void setSpriteTapFn(std::string spriteName, const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback);

    // Build => Run Animations on children
    // this->addAnimation(name, duration, delay).size(elementName, to, delay=0).opacity(elementName, to,  delay=0).finishFn(callback);
    // this->addAnimationScript(name, file-or-string)
    // this->runAnimation(name)

    // Run Animations on children immediately
    // this->animate(duration, delay)
    // this->animate(file-or-string)

protected:
    using sMap          = std::map<std::string, ds::ui::Sprite*>;
    using eventCallback = std::function<void(const ds::Event&)>;
    using eventMap      = std::unordered_map<int, eventCallback>;

    void onAppEvent(const ds::Event&);

    std::string           mLayoutFile;
    ds::EventClient       mEventClient;
    sMap                  mSpriteMap;
    eventMap              mEventCallbacks;
};

} // namespace ui
} // namespace ds

#endif
