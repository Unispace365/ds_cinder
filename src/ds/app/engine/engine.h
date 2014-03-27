#pragma once
#ifndef DS_APP_ENGINE_ENGINE_H_
#define DS_APP_ENGINE_ENGINE_H_

#include <memory>
#include <unordered_map>
#include "ds/app/app_defs.h"
#include "ds/app/auto_update_list.h"
#include "ds/app/blob_registry.h"
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
#include "ds/ui/touch/touch_manager.h"
#include "ds/ui/tween/tweenline.h"
#include "cinder/Camera.h"
#include "ds/network/zmq_connection.h"
#include "ds/data/raw_data_buffer.h"
#include "cinder/gl/Fbo.h"
#include "ds/app/camera_utils.h"

using namespace ci;
using namespace ci::app;

namespace ds {
class App;
class AutoUpdate;

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
	virtual void						requestDeleteSprite(ds::ui::Sprite&);

	tuio::Client&						getTuioClient();
	void								mouseTouchBegin(MouseEvent, int id);
	void								mouseTouchMoved(MouseEvent, int id);
	void								mouseTouchEnded(MouseEvent, int id);
	void								touchesBegin(TouchEvent);
	void								touchesMoved(TouchEvent);
	void								touchesEnded(TouchEvent);

	virtual ds::ResourceList&			getResources();
	virtual const ds::FontList&			getFonts() const;
	ds::FontList&						editFonts();

	virtual void						setCamera(const bool perspective = false);
	// Used by the perspective camera to set the near and far planes
	void								setPerspectiveCameraPlanes(const float near, const float far);

	virtual void						setPerspectiveCameraPosition(const ci::Vec3f &pos);
	virtual ci::Vec3f					getPerspectiveCameraPosition() const;
	virtual void						setPerspectiveCameraTarget(const ci::Vec3f &tar);
	virtual ci::Vec3f					getPerspectiveCameraTarget() const;
	// Modal -- prepare for the user to be able to move the camera.
	// I discovered there's a call to setViewport() when the camera is set up that *seems* to
	// do nothing except prevent me from dynamically changing the camera bounds. However, I
	// know I'll discover it's significance at some point down the road, so this just lets
	// specific apps turn off this behaviour.
	void								setToUserCamera();
	
	// Tmp
	ci::CameraPersp&					getPerspectiveCamera() { return mCameraPersp; }

	// Can be used by apps to stop services before exiting.
	// This will happen automatically, but some apps might want
	// to make sure everything is stopped before they go away.
	virtual void						stopServices();

	bool								systemMultitouchEnabled() const;
	bool								hideMouse() const;

	virtual void						clearFingers( const std::vector<int> &fingers );
	void								setSpriteForFinger( const int fingerId, ui::Sprite* theSprite ){ mTouchManager.setSpriteForFinger(fingerId, theSprite); }

	// Add or

protected:
	Engine(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const std::vector<int>* roots);

	// Conveniences for the subclases
	void								updateClient();
	void								updateServer();
	void								drawClient();
	void								drawServer();
	void								setCameraForDraw(const bool perspective = false);
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

private:
	void								deleteRequestedSprites();

	std::vector<ui::Sprite*>			mRoots;
	std::vector<ds::sprite_id_t>		mRequestDelete;

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
	bool								mDrawTouches;
	// Clients that will get update() called automatically at the start
	// of each update cycle
	AutoUpdateList						mAutoUpdate;

	ds::cfg::Settings					mDebugSettings;
	ci::CameraOrtho						mCamera;
	ci::CameraPersp						mCameraPersp;
	ci::Vec2f							mCameraZClipping;
	float								mCameraFOV;
	int									mCameraType;
	float								mCameraPerspNearPlane,
										mCameraPerspFarPlane;
	ci::Vec3f							mCameraPosition;
	ci::Vec3f							mCameraTarget;
	// See disableSetViewport()
	bool								mSetViewport;

	ci::gl::Fbo							mFbo;

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

	bool								mSystemMultitouchEnabled;
	bool								mHideMouse;

	bool								mApplyFxAA;
	float								mFxAASpanMax;
	float								mFxAAReduceMul;
	float								mFxAAReduceMin;
};

// Server -> Client communication
extern const char				CMD_SERVER_SEND_WORLD; // The server is sending the entire world

// Client -> Server communication
extern const char				CMD_CLIENT_REQUEST_WORLD; // The client is requesting the entire world

} // namespace ds

#endif // DS_APP_ENGINE_ENGINE_H_