#pragma once
#ifndef DS_UI_SPRITE_SPRITEENGINE_H_
#define DS_UI_SPRITE_SPRITEENGINE_H_
#include "cinder/Rect.h"

namespace ds {
class ResourceList;
class WorkManager;

namespace ui {
class LoadImageService;

/**
 * \class ds::ui::SpriteEngine
 * Interface for the API that is supplied to sprites.
 */
class SpriteEngine {
  public:
    virtual ds::WorkManager       &getWorkManager() = 0;
    virtual ds::ResourceList      &getResources() = 0;
    virtual ui::LoadImageService  &getLoadImageService() = 0;

    virtual float                  getMinTouchDistance() const = 0;
    virtual float                  getMinTapDistance() const = 0;
    virtual unsigned               getSwipeQueueSize() const = 0;
    virtual float                  getDoubleTapTime() const = 0;
    virtual ci::Rectf              getScreenRect() const = 0;
    virtual float                  getWidth() const = 0;
    virtual float                  getHeight() const = 0;
  protected:
    SpriteEngine()                 { }
    virtual ~SpriteEngine()        { }
};

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_