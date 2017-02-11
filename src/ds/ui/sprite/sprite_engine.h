#pragma once
#ifndef DS_UI_SPRITE_SPRITEENGINE_H_
#define DS_UI_SPRITE_SPRITEENGINE_H_
#include <list>
#include <unordered_map>
#include <cinder/Camera.h>
#include <cinder/Rect.h>
#include <cinder/Vector.h>
#include <cinder/Xml.h>
#include <cinder/app/Window.h>
#include "ds/app/app_defs.h"
#include "fbo/fbo.h"
#include <memory>

namespace ds {
class AutoUpdateList;
class ColorList;
class EngineCfg;
class EngineData;
class EngineService;
class EventNotifier;
class FontList;
class ImageRegistry;
class PerspCameraParams;
class ResourceList;
class WorkManager;
class ComputerInfo;

namespace cfg {
class Settings;
}

namespace ui {
class LoadImageService;
class PangoFontService;
class Sprite;
class Tweenline;
class TouchEvent;
struct TouchInfo;

/**
 * \class ds::ui::SpriteEngine
 * Interface for the API that is supplied to sprites.
 */
class SpriteEngine {
public:
	/** Access to the app-wide notification service. Use this to send a
	 message to everyone who's registered an EventClient. */
	ds::EventNotifier&				getNotifier();

	/** New-style notifier, access a  named channel. Create the
		 channel if it doesn't exist. */
	virtual ds::EventNotifier&		getChannel(const std::string&) = 0;

	// General engine services
	virtual ds::WorkManager&		getWorkManager() = 0;
	virtual ds::ResourceList&		getResources() = 0;
	virtual const ds::ColorList&	getColors() const = 0;
	virtual const ds::FontList&		getFonts() const = 0;
	virtual ds::AutoUpdateList&		getAutoUpdateList(const int = AutoUpdateType::SERVER) = 0;
	virtual LoadImageService&		getLoadImageService() = 0;
	virtual PangoFontService&		getPangoFontService() = 0;
	virtual ds::ImageRegistry&		getImageRegistry() = 0;
	virtual Tweenline&				getTweenline() = 0;
	virtual const ds::cfg::Settings&
									getDebugSettings() = 0;
	virtual ci::app::WindowRef		getWindow() = 0;

	bool							getMute();
	void							setMute(bool);

	/** Defined by platform:guid. Useful if you need to something specific on a particular client */
	const std::string				getAppInstanceName();

	/** Access a service. Throw if the service doesn't exist.
		Handle casting for you (since the root ds::EngineService class is unuseable). */
	template <typename T>
	T&								getService(const std::string&);
	// Answers true if the requested service exists and registered with Engine.
	bool							hasService(const std::string&) const;

	/** Access to the current engine configuration info. */
	void							loadSettings(const std::string& name, const std::string& filename);
	
	ds::EngineCfg&					getEngineCfg();
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
	float							getSwipeMinVelocity() const;
	float							getSwipeMaxTime() const;
	float							getDoubleTapTime() const;
	const ci::Rectf&				getSrcRect() const;
	const ci::Rectf&				getDstRect() const;
	// This should be obsoleted, everyone should be using src and dst rects now
	ci::Rectf						getScreenRect() const;
	float							getWidth() const;
	float							getHeight() const;
	float							getWorldWidth() const;
	float							getWorldHeight() const;
	float							getFrameRate() const;

	std::unique_ptr<FboGeneral>		getFbo();
	void							giveBackFbo(std::unique_ptr<FboGeneral> fbo);

	// Camera control. Will throw if the root at the index is the wrong type.
	// NOTE: You can't call setPerspectiveCamera() in the app constructor. Call
	// no earlier than App::setup().
	virtual PerspCameraParams		getPerspectiveCamera(const size_t index) const = 0;
	// For clients that frequently read the camera params, they can cache a direct reference.
	virtual const ci::CameraPersp&	getPerspectiveCameraRef(const size_t index) const = 0;
	virtual void					setPerspectiveCamera(const size_t index, const PerspCameraParams&) = 0;
	virtual void					setPerspectiveCameraRef(const size_t index, const ci::CameraPersp&) = 0;

	// Will throw if the root at the index is the wrong type
	virtual float					getOrthoFarPlane(const size_t index) const = 0;
	virtual float					getOrthoNearPlane(const size_t index) const = 0;
	virtual void					setOrthoViewPlanes(const size_t index, const float nearPlane, const float farPlane) = 0;

	void							addToDragDestinationList(Sprite *sprite);
	void							removeFromDragDestinationList(Sprite *sprite);
	Sprite*							getDragDestinationSprite(const ci::vec3 &globalPoint, Sprite *draggingSprite);

	double							getElapsedTimeSeconds() const;

	int								getIdleTimeout() const;
	void							setIdleTimeout(int idleTimeout);
	virtual void					resetIdleTimeout(){};
	virtual void					startIdling(){};
	// deprecated -- use resetIdleTimeout()
	virtual void					resetIdleTimeOut() { resetIdleTimeout(); }

	virtual void					clearFingers( const std::vector<int> &fingers );
	virtual void					setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ) = 0;
	virtual ui::Sprite*				getSpriteForFinger( const int fingerId ) = 0;

	// If you want to create touch events from your client app, use these functions.
	// The touch events will use the same pathways that normal touches would.
	// This is generally only recommended for debugging stuff (like automators) 
	// or if you have an unusual input situation (like a kinect or something) and want to use touch
	virtual void					injectTouchesBegin(const ds::ui::TouchEvent&) = 0;
	virtual void					injectTouchesMoved(const ds::ui::TouchEvent&) = 0;
	virtual void					injectTouchesEnded(const ds::ui::TouchEvent&) = 0;

	// translate a touch event point to the overlay bounds specified in the settings
	virtual void					translateTouchPoint( ci::vec2& inOutPoint ) = 0;

	/// Calls every time any touch anywhere happens, and the touch info is post-translation and filtering
	/// This calls *after* any sprites get the touch. 
	void							setTouchInfoPipeCallback(std::function<void(const ds::ui::TouchInfo&)> func){ mTouchInfoPipe = func; }
	
	/// Get the function for touch info callbacks, for TouchManager to callback on.
	std::function<void(const ds::ui::TouchInfo&)>	getTouchInfoPipeCallback(){ return mTouchInfoPipe; }

	// Turns on Sprite's setRotateTouches when first created so you can enable rotated touches app-wide by default
	// Sprites can still turn this off after creation
	virtual bool					getRotateTouchesDefault() = 0;

	// Get the sprite at the global touch point. NOTE: performance intensive. Use carefully.
	virtual ds::ui::Sprite*			getHit(const ci::vec3& point) = 0;


	static const int				CLIENT_MODE = 0;
	static const int				SERVER_MODE = 1;
	static const int				CLIENTSERVER_MODE = 2;
	static const int				STANDALONE_MODE = 3;
	virtual int						getMode() const = 0;

	ds::ComputerInfo&				getComputerInfo();

	/** Register a function to a sprite type. This allows an xml sprite importer to create sprites it knows nothing about, like Jon Snow. */
	void							registerSpriteImporter(const std::string& spriteType, std::function<ds::ui::Sprite*(ds::ui::SpriteEngine&)> func);
	/** Create a sprite of a type specified by the spriteType name in registerSpriteImporter(). Can return nullptr if there's no sprite registered for that name. */
	ds::ui::Sprite*					createSpriteImporter(const std::string& spriteType);

	/** Register a callback to set the property of a sprite during import by an outside caller (like an xml importer) */
	void							registerSpritePropertySetter(const std::string& propertyName, std::function<void(ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileRefferer)> func);

	/** Set the property of a sprite by name and value string. File referrer (optional) is the relative file path to look up files. See ds/util/file_meta_data.h for relative path finding */
	bool							setRegisteredSpriteProperty(const std::string& propertyName, ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileRefferer = "");

protected:
	// The data is not copied, so it needs to exist for the life of the SpriteEngine,
	// which is how things work by default (the data and engine are owned by the App).
	SpriteEngine(ds::EngineData&);
	virtual ~SpriteEngine();

	ds::EngineData&					mData;
	std::list<Sprite *>				mDragDestinationSprites;
	ds::ComputerInfo*				mComputerInfo;

	std::list<std::unique_ptr<FboGeneral>> mFbos;

	std::unordered_map<std::string, std::function<ds::ui::Sprite*(ds::ui::SpriteEngine&)>> mImporterMap;
	std::unordered_map<std::string, std::function<void(ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileRefferer)>> mPropertyMap;

private:
	ds::EngineService&				private_getService(const std::string&);

	std::function<void(const ds::ui::TouchInfo& ti)>	mTouchInfoPipe;
};

template <typename T>
T& SpriteEngine::getService(const std::string& str) {
	return dynamic_cast<T&>(private_getService(str));
}

} // namespace ui

} // namespace ds

#endif // DS_APP_ENGINE_H_
