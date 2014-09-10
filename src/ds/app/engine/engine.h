#pragma once
#ifndef DS_APP_ENGINE_ENGINE_H_
#define DS_APP_ENGINE_ENGINE_H_

#include <memory>
#include <unordered_map>
#include <cinder/Camera.h>
#include <cinder/gl/Fbo.h>
#include "ds/app/app_defs.h"
#include "ds/app/auto_update_list.h"
#include "ds/app/blob_registry.h"
#include "ds/app/event_notifier.h"
#include "ds/app/image_registry.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/params/update_params.h"
#include "ds/params/draw_params.h"
#include <cinder/app/App.h>
#include <cinder/app/TouchEvent.h>
#include <cinder/app/AppBasic.h>
#include "TuioClient.h"
#include "ds/app/engine/engine_touch_queue.h"
#include "ds/data/font_list.h"
#include "ds/data/resource_list.h"
#include "ds/data/tuio_object.h"
#include "ds/cfg/settings.h"
#include "ds/ui/ip/ip_function_list.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/touch/select_picking.h"
#include "ds/ui/touch/touch_manager.h"
#include "ds/ui/touch/touch_translator.h"
#include "ds/ui/tween/tweenline.h"
#include "ds/network/zmq_connection.h"
#include "ds/data/raw_data_buffer.h"
#include "ds/app/camera_utils.h"

using namespace ci;
using namespace ci::app;

namespace ds {
class App;
class AutoDrawService;
class AutoUpdate;
class EngineRoot;

extern const ds::BitMask	ENGINE_LOG;

/**
 * \class ds::Engine
 * \brief Concrete implementation of the SpriteEngine. Contain all the
 * behind-the-scenes pieces necessary for running the app. NOTE: This
 * class should be internal to the framework. All clients should know
 * about is SpriteEngine.
 */
class Engine : public ui::SpriteEngine {
public:
	static const int					CAMERA_ORTHO = 0;
	static const int					CAMERA_PERSP = 1;

	~Engine();

	virtual void						update() = 0;
	virtual void						draw() = 0;

	virtual ds::EventNotifier&			getChannel(const std::string&);
	void								addChannel(const std::string &name, const std::string &description);
	virtual ds::AutoUpdateList&			getAutoUpdateList() { return mAutoUpdate; }
	virtual ds::ImageRegistry&			getImageRegistry() { return mImageRegistry; }
	virtual ds::ui::Tweenline&			getTweenline() { return mTweenline; }
	virtual const ds::cfg::Settings&	getDebugSettings() { return mDebugSettings; }
	// I take ownership of any services added to me.
	void								addService(const std::string&, ds::EngineService&);
	// Add the image processing function to my global pool
	void								addIp(const std::string& key, const ds::ui::ip::FunctionRef&);
	// Convenice to load a setting file into the mEngineCfg settings.
	// @param name is the name that the system will use to refer to the settings.
	// @param filename is the leaf path of the settings file (i.e. "data.xml").
	// It will be loaded from all appropriate locations.
	void								loadSettings(const std::string& name, const std::string& filename);
	// Convenice to load a text cfg file into a collection of cfg objects.
	// @param filename is the leaf path of the settings file (i.e. "text.xml").
	// It will be loaded from all appropriate locations.
	void								loadTextCfg(const std::string& filename);
	// Convenice to load a nine patch cfg file into a collection of cfg objects.
	// @param filename is the leaf path of the settings file (i.e. "nine_patch.xml").
	// It will be loaded from all appropriate locations.
	void								loadNinePatchCfg(const std::string& filename);

	const ds::EngineData&				getEngineData() const		{ return mData; }
	// only valid after setup() is called
	int									getRootCount() const;
	ui::Sprite&							getRootSprite(const size_t index = 0);

	void								prepareSettings( ci::app::AppBasic::Settings& );
	//called in app setup; loads settings files and what not.
	virtual void						setup(ds::App&);
	virtual void						setupTuio(ds::App&) = 0;

	bool								isIdling() const;
	void								startIdling();
	void								resetIdleTimeOut();
    
	// Called during app construction, to register the sprites as blob handlers.
	virtual void						installSprite(	const std::function<void(ds::BlobRegistry&)>& asServer,
														const std::function<void(ds::BlobRegistry&)>& asClient) = 0;

	virtual ds::sprite_id_t				nextSpriteId();
	virtual void						registerSprite(ds::ui::Sprite&);
	virtual void						unregisterSprite(ds::ui::Sprite&);
	virtual ds::ui::Sprite*				findSprite(const ds::sprite_id_t);
	virtual void						spriteDeleted(const ds::sprite_id_t&);
	virtual ci::Color8u					getUniqueColor();

	tuio::Client&						getTuioClient();
	void								touchesBegin(const TouchEvent&);
	void								touchesMoved(const TouchEvent&);
	void								touchesEnded(const TouchEvent&);
	void								mouseTouchBegin(const MouseEvent&, int id);
	void								mouseTouchMoved(const MouseEvent&, int id);
	void								mouseTouchEnded(const MouseEvent&, int id);
	MouseEvent							alteredMouseEvent(const MouseEvent&) const;

	virtual ds::ResourceList&			getResources();
	virtual const ds::FontList&			getFonts() const;
	ds::FontList&						editFonts();

	void								markCameraDirty();
	virtual PerspCameraParams			getPerspectiveCamera(const size_t index) const;
	virtual const ci::CameraPersp&		getPerspectiveCameraRef(const size_t index) const;
	virtual void						setPerspectiveCamera(const size_t index, const PerspCameraParams&);

	// Modal -- prepare for the user to be able to move the camera.
	// I discovered there's a call to setViewport() when the camera is set up that *seems* to
	// do nothing except prevent me from dynamically changing the camera bounds. However, I
	// know I'll discover it's significance at some point down the road, so this just lets
	// specific apps turn off this behaviour.
	void								setToUserCamera();

	// Can be used by apps to stop services before exiting.
	// This will happen automatically, but some apps might want
	// to make sure everything is stopped before they go away.
	virtual void						stopServices();

	bool								hideMouse() const;

	ds::ui::Sprite*						getHit(const ci::Vec3f& point);

	virtual void						clearFingers( const std::vector<int> &fingers );
	void								setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ){ mTouchManager.setSpriteForFinger(fingerId, theSprite); }
	ds::ui::Sprite*						getSpriteForFinger( const int fingerId ){ return mTouchManager.getSpriteForFinger(fingerId); }
	// translate a touch event point to the overlay bounds specified in the settings
	virtual void						translateTouchPoint( ci::Vec2f& inOutPoint );
	virtual bool						shouldDiscardTouch( ci::Vec2f& p ){ return mTouchManager.shouldDiscardTouch(p); }

	// Root support
	const ci::Rectf&					getScreenRect() const;
	// Utility to change touch mode
	void								nextTouchMode();

	// Debugging aid to write out the sprites
	void								writeSprites(std::ostream&) const;

protected:
	Engine(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const RootList&);

	// Conveniences for the subclases
	void								updateClient();
	void								updateServer();
	void								drawClient();
	void								drawServer();
	// Called from the destructor of all subclasses, so I can cleanup
	// sprites before services go away.
	void								clearAllSprites();
	void								registerForTuioObjects(tuio::Client&);

	static const int					NumberOfNetworkThreads;

	ds::BlobRegistry					mBlobRegistry;
	std::unordered_map<ds::sprite_id_t, ds::ui::Sprite*>
										mSprites;
	int									mTuioPort;

	// All the installed image processing functions.
	ds::ui::ip::FunctionList			mIpFunctions;
	ds::ui::TouchMode::Enum				mTouchMode;

private:
	// Special function to set the camera to the current screen and clear it.
	void								clearScreen();
	void								setTouchMode(const ds::ui::TouchMode::Enum&);

	friend class EngineStatsView;
	std::vector<std::unique_ptr<EngineRoot>>
										mRoots;
	const ds::cfg::Settings&			mSettings;
	ImageRegistry						mImageRegistry;
	ds::ui::Tweenline					mTweenline;
	// A cache of all the resources in the system
	ResourceList						mResources;
	FontList							mFonts;
	UpdateParams						mUpdateParams;
	DrawParams							mDrawParams;
	float								mLastTime;
	bool								mIdling;
	float								mLastTouchTime;
	float								mIdleTime;

	tuio::Client						mTuio;
	ui::TouchManager					mTouchManager;
	// Clients that will get update() called automatically at the start
	// of each update cycle
	AutoUpdateList						mAutoUpdate;
	// Quick hack to get any ol' client participating in draw
	AutoDrawService*					mAutoDraw;

	ds::cfg::Settings					mDebugSettings;

	ci::gl::Fbo							mFbo;

	ds::ui::TouchTranslator				mTouchTranslator;
	std::mutex							mTouchMutex;
	ds::EngineTouchQueue<TouchEvent>	mTouchBeginEvents;
	ds::EngineTouchQueue<TouchEvent>	mTouchMovedEvents;
	ds::EngineTouchQueue<TouchEvent>	mTouchEndEvents;
	typedef std::pair<MouseEvent, int> MousePair;
	ds::EngineTouchQueue<MousePair>		mMouseBeginEvents;
	ds::EngineTouchQueue<MousePair>		mMouseMovedEvents;
	ds::EngineTouchQueue<MousePair>		mMouseEndEvents;
	// Only used if the settings file has "tuio:receive_objects" set to true
	ds::EngineTouchQueue<TuioObject>	mTuioObjectsBegin;
	ds::EngineTouchQueue<TuioObject>	mTuioObjectsMoved;
	ds::EngineTouchQueue<TuioObject>	mTuioObjectsEnd;

	ds::SelectPicking					mSelectPicking;

	bool								mHideMouse;

	bool								mApplyFxAA;
	float								mFxAASpanMax;
	float								mFxAAReduceMul;
	float								mFxAAReduceMin;
	ci::Color8u							mUniqueColor;

	int									mCachedWindowW, mCachedWindowH;

	// Channels. A channel is simply a notifier, with an optional description.
	class Channel {
	public:
		Channel();
		Channel(const std::string &description);

		ds::EventNotifier				mNotifier;
		std::string						mDescription;
	};
	std::unordered_map<std::string, Channel>
										mChannels;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINE_H_