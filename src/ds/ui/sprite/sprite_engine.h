#pragma once
#ifndef DS_UI_SPRITE_SPRITEENGINE_H_
#define DS_UI_SPRITE_SPRITEENGINE_H_
#include <list>
#include <unordered_map>
#include <cinder/Camera.h>
#include <cinder/Rect.h>
#include <cinder/Vector.h>
#include <cinder/app/Window.h>
#include "ds/app/app_defs.h"
#include "fbo/fbo.h"
#include <memory>

namespace ds {
class AutoUpdateList;
class EngineCfg;
class EngineData;
class EngineService;
class EventNotifier;
class FontList;
class ImageRegistry;
class PerspCameraParams;
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
	// Access to the app-wide notification service. Use this to send a
	// message to everyone who's registered an EventClient.
	ds::EventNotifier&				getNotifier();
	// New-style notifier, access a  named channel. Create the
	// channel if it doesn't exist.
	virtual ds::EventNotifier&		getChannel(const std::string&) = 0;

	// General engine services
	virtual ds::WorkManager&		getWorkManager() = 0;
	virtual ds::ResourceList&		getResources() = 0;
	virtual const ds::FontList&		getFonts() const = 0;
	virtual ds::AutoUpdateList&		getAutoUpdateList(const int = AutoUpdateType::SERVER) = 0;
	virtual LoadImageService&		getLoadImageService() = 0;
	virtual RenderTextService&		getRenderTextService() = 0;
	virtual ds::ImageRegistry&		getImageRegistry() = 0;
	virtual Tweenline&				getTweenline() = 0;
	virtual const ds::cfg::Settings&
									getDebugSettings() = 0;
	virtual ci::app::WindowRef		getWindow() = 0;

	// Access a service. Throw if the service doesn't exist.
	// Handle casting for you (since the root ds::EngineService class
	// is unuseable).
	template <typename T>
	T&								getService(const std::string&);

	// Access to the current engine configuration info.
	const ds::EngineCfg&			getEngineCfg() const;
	// Shortcuts
	const ds::cfg::Settings&		getSettings(const std::string& name) const;

	// Sprite management
	virtual ds::sprite_id_t			nextSpriteId() = 0;
	virtual void					registerSprite(Sprite&) = 0;
	virtual void					unregisterSprite(Sprite&) = 0;
	virtual Sprite*					findSprite(const ds::sprite_id_t) = 0;
	// Notification that a sprite has been deleted
	virtual void					spriteDeleted(const ds::sprite_id_t&) = 0;
	virtual ci::Color8u				getUniqueColor() = 0;

	float							getMinTouchDistance() const;
	float							getMinTapDistance() const;
	unsigned						getSwipeQueueSize() const;
	float							getDoubleTapTime() const;
	ci::Rectf						getScreenRect() const;
	float							getWidth() const;
	float							getHeight() const;
	float							getWorldWidth() const;
	float							getWorldHeight() const;
	float							getFrameRate() const;

	std::unique_ptr<FboGeneral>		getFbo();
	void							giveBackFbo(std::unique_ptr<FboGeneral> &fbo);

	// Camera control. Will throw if the root at the index is the wrong type.
	// NOTE: You can't call setPerspectiveCamera() in the app constructor. Call
	// no earlier than App::setup().
	virtual PerspCameraParams			getPerspectiveCamera(const size_t index) const = 0;
	// For clients that frequently read the camera params, they can cache a direct reference.
	virtual const ci::CameraPersp&		getPerspectiveCameraRef(const size_t index) const = 0;
	virtual void						setPerspectiveCamera(const size_t index, const PerspCameraParams&) = 0;

	void							addToDragDestinationList(Sprite *sprite);
	void							removeFromDragDestinationList(Sprite *sprite);
	Sprite*							getDragDestinationSprite(const ci::Vec3f &globalPoint, Sprite *draggingSprite);

	double							getElapsedTimeSeconds() const;

	virtual void					clearFingers( const std::vector<int> &fingers );
	virtual void					setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ) = 0;
	virtual ui::Sprite*				getSpriteForFinger( const int fingerId ) = 0;

	// If you want to create touch events from your client app, use these functions.
	// The touch events will use the same pathways that normal touches would.
	// This is generally only recommended for debugging stuff (like automators) 
	// or if you have an unusual input situation (like a kinect or something) and want to use touch
	virtual void					injectTouchesBegin(const ci::app::TouchEvent&){};
	virtual void					injectTouchesMoved(const ci::app::TouchEvent&){};
	virtual void					injectTouchesEnded(const ci::app::TouchEvent&){};

	// translate a touch event point to the overlay bounds specified in the settings
	virtual void					translateTouchPoint( ci::Vec2f& inOutPoint ) = 0;

	// Get the sprite at the global touch point. NOTE: performance intensive. Use carefully.
	virtual ds::ui::Sprite*			getHit(const ci::Vec3f& point) = 0;


	static const int				CLIENT_MODE = 0;
	static const int				SERVER_MODE = 1;
	static const int				CLIENTSERVER_MODE = 2;
	static const int				STANDALONE_MODE = 3;
	virtual int						getMode() const = 0;

protected:
	// The data is not copied, so it needs to exist for the life of the SpriteEngine,
	// which is how things work by default (the data and engine are owned by the App).
	SpriteEngine(ds::EngineData&);
	virtual ~SpriteEngine();

	ds::EngineData&					mData;
	std::list<Sprite *>				mDragDestinationSprites;

	std::list<std::unique_ptr<FboGeneral>> mFbos;

private:
	ds::EngineService&				private_getService(const std::string&);
};

template <typename T>
T& SpriteEngine::getService(const std::string& str) {
	return dynamic_cast<T&>(private_getService(str));
}

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_