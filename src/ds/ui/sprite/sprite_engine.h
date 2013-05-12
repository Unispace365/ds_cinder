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

namespace Awesomium {
class WebCore;
class WebSession;
}

namespace ds {
class AutoUpdateList;
class FontList;
class ImageRegistry;
class ResourceList;
class WorkManager;

namespace cfg {
class Settings;
}

namespace ui {
class LoadImageService;
class RenderTextService;
class Sprite;
class Tweenline;

/**
 * \class ds::ui::SpriteEngine
 * Interface for the API that is supplied to sprites.
 */
class SpriteEngine {
public:
	// General engine services
	virtual ds::WorkManager&	getWorkManager() = 0;
	virtual ds::ResourceList&	getResources() = 0;
	virtual const ds::FontList&	getFonts() const = 0;
	virtual ds::AutoUpdateList&	getAutoUpdateList() = 0;
	virtual LoadImageService&	getLoadImageService() = 0;
	virtual RenderTextService&	getRenderTextService() = 0;
	virtual ds::ImageRegistry&	getImageRegistry() = 0;
	virtual Tweenline&			getTweenline() = 0;
	virtual const ds::cfg::Settings&
								getDebugSettings() = 0;

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
    float													 getFrameRate() const;

    std::unique_ptr<FboGeneral>    getFbo();
    void                           giveBackFbo(std::unique_ptr<FboGeneral> &fbo);

    virtual void                   setCamera(const bool perspective = false) = 0;
    // Camera control
    virtual void                   setPerspectiveCameraPosition(const ci::Vec3f &pos) = 0;
    virtual ci::Vec3f              getPerspectiveCameraPosition() const = 0;
    virtual void                   setPerspectiveCameraTarget(const ci::Vec3f &tar) = 0;
    virtual ci::Vec3f              getPerspectiveCameraTarget() const = 0;

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

    virtual Awesomium::WebCore    *getWebCore() const = 0;
    virtual Awesomium::WebSession *getWebSession() const = 0;

	protected:
    SpriteEngine();
    virtual ~SpriteEngine()        { }

    std::list<Sprite *>            mDragDestinationSprites;

    std::list<std::unique_ptr<FboGeneral>> mFbos;
		
		// This is because of the growing number of virtuals, which of course impacts
		// performance. We should probably unvirtualize all the trivial data types.
		float													mFrameRate;
};

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_