#pragma once
#ifndef DS_UI_SPRITE_SPRITEENGINE_H_
#define DS_UI_SPRITE_SPRITEENGINE_H_
#include "cinder/Rect.h"
#include <list>
#include "cinder/Vector.h"
#include "ds/app/app_defs.h"

namespace ds {
class ResourceList;
class WorkManager;

namespace ui {
class LoadImageService;
class Sprite;

/**
 * \class ds::ui::SpriteEngine
 * Interface for the API that is supplied to sprites.
 */
class SpriteEngine {
  public:
    // General engine services
    virtual ds::WorkManager       &getWorkManager() = 0;
    virtual ds::ResourceList      &getResources() = 0;
    virtual ui::LoadImageService  &getLoadImageService() = 0;

    // Sprite management
    virtual ds::sprite_id_t        nextSpriteId() = 0;
    virtual void                   registerSprite(Sprite&) = 0;
    virtual void                   unregisterSprite(Sprite&) = 0;
    virtual Sprite*                findSprite(const ds::sprite_id_t) = 0;

    virtual float                  getMinTouchDistance() const = 0;
    virtual float                  getMinTapDistance() const = 0;
    virtual unsigned               getSwipeQueueSize() const = 0;
    virtual float                  getDoubleTapTime() const = 0;
    virtual ci::Rectf              getScreenRect() const = 0;
    virtual float                  getWidth() const = 0;
    virtual float                  getHeight() const = 0;
    virtual float                  getWorldWidth() const = 0;
    virtual float                  getWorldHeight() const = 0;

    void                           addToDragDestinationList(Sprite *sprite);
    void                           removeFromDragDestinationList(Sprite *sprite);
    Sprite                        *getDragDestinationSprite(const ci::Vec3f &globalPoint, Sprite *draggingSprite);
  protected:
    SpriteEngine()                 { }
    virtual ~SpriteEngine()        { }

    std::list<Sprite *>            mDragDestinationSprites;
};

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_