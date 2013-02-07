#pragma once
#ifndef DS_UI_SPRITE_SPRITEENGINE_H_
#define DS_UI_SPRITE_SPRITEENGINE_H_
#include "cinder/Rect.h"
#include <list>
#include <unordered_map>
#include "cinder/Vector.h"
#include "ds/app/app_defs.h"
#include "fbo/fbo.h"
#include <memory>

namespace ds {
class AutoUpdateList;
class ResourceList;
class WorkManager;

namespace cfg {
class Settings;
}

namespace ui {
class LoadImageService;
class Sprite;
class Tweenline;

/**
 * \class ds::ui::SpriteEngine
 * Interface for the API that is supplied to sprites.
 */
class SpriteEngine {
  public:
    // General engine services
    virtual ds::WorkManager       &getWorkManager() = 0;
    virtual ds::ResourceList      &getResources() = 0;
    virtual ds::AutoUpdateList    &getAutoUpdateList() = 0;
    virtual LoadImageService      &getLoadImageService() = 0;
    virtual Tweenline             &getTweenline() = 0;
    virtual const ds::cfg::Settings
                                  &getDebugSettings() = 0;

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
    
    std::unique_ptr<FboGeneral>    getFbo();
    void                           giveBackFbo(std::unique_ptr<FboGeneral> &fbo);

    virtual void                   setCamera() = 0;

    void                           addToDragDestinationList(Sprite *sprite);
    void                           removeFromDragDestinationList(Sprite *sprite);
    Sprite                        *getDragDestinationSprite(const ci::Vec3f &globalPoint, Sprite *draggingSprite);

    double                         getElapsedTimeSeconds() const;

	virtual void                   clearFingers( const std::vector<int> &fingers );
	virtual void				   setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ) = 0;

    static const int               CLIENT_MODE = 0;
    static const int               SERVER_MODE = 1;
    static const int               CLIENTSERVER_MODE = 2;
    virtual int                    getMode() const = 0;

  protected:
    SpriteEngine()                 { }
    virtual ~SpriteEngine()        { }

    std::list<Sprite *>            mDragDestinationSprites;

    std::list<std::unique_ptr<FboGeneral>> mFbos;
};

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_