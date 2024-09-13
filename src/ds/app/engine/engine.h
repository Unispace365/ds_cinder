#pragma once

#include <memory>
#include <unordered_map>

#include <cinder/Camera.h>
#include <cinder/app/App.h>
#include <cinder/app/TouchEvent.h>
#include <cinder/gl/Fbo.h>

#include "ds/app/app_defs.h"
#include "ds/app/auto_update_list.h"
#include "ds/app/blob_registry.h"
#include "ds/app/engine/engine_settings.h"
#include "ds/app/engine/engine_touch_queue.h"
#include "ds/app/event_client.h"
#include "ds/app/event_notifier.h"
#include "ds/app/image_registry.h"
#include "ds/data/color_list.h"
#include "ds/data/font_list.h"
#include "ds/data/resource_list.h"
#include "ds/data/tuio_object.h"
#include "ds/debug/auto_refresh.h"
#include "ds/params/draw_params.h"
#include "ds/params/update_params.h"
#include "ds/ui/service/pango_font_service.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/touch/touch_manager.h"
#include "ds/ui/touch/touch_translator.h"

#include "ds/app/camera_utils.h"
#include "ds/ui/tween/tweenline.h"

namespace cinder::tuio {
class Receiver;
}

namespace ds {
class App;
class AutoDrawService;
class AutoUpdate;
class EngineRoot;
} // namespace ds

namespace ds::ui {
class TuioInput;
class LoadImageService;
} // namespace ds::ui

namespace ds::cfg {
class SettingsEditor;
class Text;
} // namespace ds::cfg


namespace ds {
extern const ds::BitMask ENGINE_LOG;
/**
 * \class Engine
 * \brief Concrete implementation of the SpriteEngine. Contain all the
 * behind-the-scenes pieces necessary for running the app. NOTE: This
 * class should be internal to the framework. All clients should know
 * about is SpriteEngine.
 */
class Engine : public ui::SpriteEngine {
  public:
	static const int CAMERA_ORTHO = 0;
	static const int CAMERA_PERSP = 1;

	~Engine();

	virtual void update() = 0;
	virtual void draw()	  = 0;

	void notifyOnChannel(const ds::Event& event, const std::string& channel, bool defaultAlso = false);
	void notifyOnChannels(const ds::Event& event, std::initializer_list<std::string> channels, bool defaultAlso=false);
	virtual ds::EventNotifier&		  getChannel(const std::string&) override;
	void							  addChannel(const std::string& name, const std::string& description);
	virtual ds::AutoUpdateList&		  getAutoUpdateList(const int = AutoUpdateType::SERVER) override;
	virtual ds::ui::PangoFontService& getPangoFontService() override { return mPangoFontService; }
	virtual ds::ui::LoadImageService& getLoadImageService() override { return *mLoadImageService; }
	virtual ds::ui::Tweenline&		  getTweenline() override { return mTweenline; }

	/// I take ownership of any services added to me.
	void addService(const std::string&, ds::EngineService&);

	/// Convenience to load a setting file into the mEngineCfg settings.
	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the leaf path of the settings file (i.e. "data.xml").
	/// It will be loaded from all appropriate locations.
	void loadSettings(const std::string& name, const std::string& filename);

	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the leaf path of the settings file (i.e. "data.xml").
	/// It will be saved ONLY in the user settings location.
	void saveSettings(const std::string& name, const std::string& filename);

	/// Convenience to append a setting file into the existing mEngineCfg settings.
	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the FULL path of the settings file (i.e. "C:\projects\settings\data.xml").
	/// It will NOT be loaded from all appropriate locations.
	void appendSettings(const std::string& name, const std::string& filename);

	/// Convenience to load a text cfg file into a collection of cfg objects.
	/// \param filename is the leaf path of the settings file (i.e. "text.xml").
	/// It will be loaded from all appropriate locations.
	void loadTextCfg(const std::string& filename);

	const ds::EngineData& getEngineData() const { return mData; }
	/// only valid after setup() is called
	size_t		getRootCount() const;
	ui::Sprite& getRootSprite(const size_t index = 0);
	/// Returns nullptr if the root sprite doesn't exist
	ui::Sprite* getRootSpritePtr(const size_t index = 0);
	/// Access to the configuration settings that created a root. Allows you to inspect pick style, debug drawing,
	/// perspective, etc
	const RootList::Root& getRootBuilder(const size_t index = 0);

	void prepareSettings(ci::app::AppBase::Settings&);
	void reloadSettings();
	void toggleSettingsEditor(const std::string& name = "");
	void showSettingsEditor(const std::string& name = "");
	void hideSettingsEditor();
	bool isShowingSettingsEditor();

	/// Called in app setup; loads settings files and what not.
	virtual void setup(ds::App&);
	void		 setupTouch(ds::App&);
	void		 startTuio(ds::App&);
	void		 stopTuio();

	/// Returns whether idle events and checks are enabled.
	bool isIdlingEnabled() const override { return mIdlingEnabled; }

	/// Sets whether idle events and checks are enabled.
	void enableIdling(bool enable) override { mIdlingEnabled = enable; }

	/// It's been enough time since the last input and is in idle mode
	virtual bool isIdling() override;

	/// Checks if it's been enough time since the last input to go into idle. Will take effect if it's been enough time
	void checkIdle();

	/// Starts idle mode right away, regardless of time
	virtual void startIdling() override;

	/// Ends idle mode, regardless of input and starts the timeout again
	virtual void stopIdling() { resetIdleTimeout(); }

	/// Identical to stopIdling(), retained for backwards compatibility
	virtual void resetIdleTimeout() override;

	/// Called during app construction, to register the sprites as blob handlers.
	virtual void installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
							   const std::function<void(ds::BlobRegistry&)>& asClient) = 0;

	virtual ds::sprite_id_t nextSpriteId() override;
	virtual void			registerSprite(ds::ui::Sprite&) override;
	virtual void			unregisterSprite(ds::ui::Sprite&) override;
	virtual ds::ui::Sprite* findSprite(const ds::sprite_id_t) override;
	virtual void			spriteDeleted(const ds::sprite_id_t&) override;
	virtual ci::Color8u		getUniqueColor() override;

	std::shared_ptr<ci::tuio::Receiver> getTuioClient(const int tuioIndex = -1);
	void								touchesBegin(const ds::ui::TouchEvent&);
	void								touchesMoved(const ds::ui::TouchEvent&);
	void								touchesEnded(const ds::ui::TouchEvent&);
	void								mouseTouchBegin(const ci::app::MouseEvent&, int id);
	void								mouseTouchMoved(const ci::app::MouseEvent&, int id);
	void								mouseTouchEnded(const ci::app::MouseEvent&, int id);
	ci::app::MouseEvent					alteredMouseEvent(const ci::app::MouseEvent&) const;

	/// If you want to create touch events from your client app, use these functions.
	/// The touch events will use the same pathways that normal touches would.
	/// This is generally only recommended for debugging stuff (like automators)
	/// or if you have an unusual input situation (like a kinect or something) and want to use touch
	/// These are separate functions from the touchesBegin, etc from above so the general
	/// use functions are not virtual and to indicate that these touchpoints are not coming from hardware
	virtual void injectTouchesBegin(const ds::ui::TouchEvent&) override;
	virtual void injectTouchesMoved(const ds::ui::TouchEvent&) override;
	virtual void injectTouchesEnded(const ds::ui::TouchEvent&) override;

	virtual void injectObjectsBegin(const ds::TuioObject&) override;
	virtual void injectObjectsMoved(const ds::TuioObject&) override;
	virtual void injectObjectsEnded(const ds::TuioObject&) override;

	/// Register a tuio::Receiver to send TUIO objects events through the Engine.  Useful if your app needs
	/// additional tuio::Receiver object listeners beyond the single tuio::Receiver provided by the Engine.
	void registerForTuioObjects(std::shared_ptr<ci::tuio::Receiver>);

	/// Turns on Sprite's setRotateTouches when first created so you can enable rotated touches app-wide by default
	/// Sprites can still turn this off after creation
	virtual bool getRotateTouchesDefault() override;

	virtual ds::ResourceList&	getResources() override;
	virtual const ds::FontList& getFonts() const override;
	ds::FontList&				editFonts();

	virtual const ds::ColorList& getColors() const override;
	virtual ds::ColorList&		 getColors() override;
	ds::ColorList&				 editColors();

	void						   markCameraDirty();
	virtual PerspCameraParams	   getPerspectiveCamera(const size_t index) const override;
	virtual const ci::CameraPersp& getPerspectiveCameraRef(const size_t index) const override;
	virtual void				   setPerspectiveCamera(const size_t index, const PerspCameraParams&) override;
	virtual void				   setPerspectiveCameraRef(const size_t index, const ci::CameraPersp&) override;

	virtual float getOrthoFarPlane(const size_t index) const override;
	virtual float getOrthoNearPlane(const size_t index) const override;
	virtual void  setOrthoViewPlanes(const size_t index, const float nearPlane, const float farPlane) override;

	/// Can be used by apps to stop services before exiting.
	/// This will happen automatically, but some apps might want
	/// to make sure everything is stopped before they go away.
	virtual void stopServices();

	void setHideMouse(const bool doMouseHide);
	bool getHideMouse() const;
	bool getAutoHideMouse() const { return mAutoHideMouse; }

	ds::ui::Sprite* getHit(const ci::vec3& point) override;

	ui::TouchManager& getTouchManager() { return mTouchManager; }
	virtual void	  clearFingers(const std::vector<int>& fingers) override;
	virtual void	  clearFingersForSprite(ui::Sprite* theSprite) override { mTouchManager.clearFingersForSprite(theSprite); }
	void			  setSpriteForFinger(const int fingerId, ui::Sprite* theSprite) override {
		 mTouchManager.setSpriteForFinger(fingerId, theSprite);
	}
	ds::ui::Sprite* getSpriteForFinger(const int fingerId) override { return mTouchManager.getSpriteForFinger(fingerId); }
	virtual bool	shouldDiscardTouch(ci::vec2& p) { return mTouchManager.shouldDiscardTouch(p); }

	void	   setTouchSmoothing(const bool doSmoothing);
	const bool getTouchSmoothing();
	void	   setTouchSmoothFrames(const int smoothFrames);

	/// Utility to change touch mode
	void nextTouchMode();

	/// Debugging aid to write out the sprites
	void writeSprites(std::ostream&) const;

	virtual ci::app::WindowRef getWindow() override;

	void toggleConsole();
	void showConsole();
	void hideConsole();

	/// Should only be used by the app class to record the average fps.
	/// Allows for debug drawing of the fps
	void		setAverageFps(const float fps) { mAverageFps = fps; }
	const float getAverageFps() const { return mAverageFps; }

	size_t getNumberOfSprites() { return mSprites.size(); }

	/// -------------------------------------------------------------
	/// These functions are inlined, since they are called frequently
	/// -------------------------------------------------------------
	/// Returns the list of current roots
	inline const std::vector<std::unique_ptr<EngineRoot>>& getRoots() const { return mRoots; }
	inline const ds::DrawParams&						   getDrawParams() const { return mDrawParams; }
	inline ds::AutoDrawService* const					   getAutoDrawService() { return mAutoDraw; }

	/// This is for Clients to reconstruct roots when they re-connect with the server
	void clearRoots();

	/// For Clients to create roots when reconnecting to the server
	void createClientRoots(std::vector<RootList::Root> newRoots);


	/** Called from the destructor of all subclasses, so I can cleanup sprites before services go away.
	\param clearDebug If true, will clear all the children from the debug roots too.
	If false, leaves them alone (for instance, in client situations) */
	void clearAllSprites(const bool clearDebug = true);

  protected:
	Engine(ds::App&, ds::EngineSettings&, ds::EngineData&, const RootList&, const int appMode);

	/// Conveniences for the subclases
	void updateClient();
	void updateServer();
	void drawClient();
	void drawServer();

	/** When mouse events are ready to be handled by the touch manager.
		These are enforced virtual functions to be sure the engine handles mouse events.
		Servers will send directly to the touch manager, and clients can send back to the server */
	virtual void handleMouseTouchBegin(const ci::app::MouseEvent&, int id) = 0;
	virtual void handleMouseTouchMoved(const ci::app::MouseEvent&, int id) = 0;
	virtual void handleMouseTouchEnded(const ci::app::MouseEvent&, int id) = 0;


	ui::TouchManager mTouchManager;

	static const int NumberOfNetworkThreads;

	ds::BlobRegistry									 mBlobRegistry;
	std::unordered_map<ds::sprite_id_t, ds::ui::Sprite*> mSprites;
	int													 mTuioPort;

	ds::ui::TouchMode::Enum mTouchMode;

  private:
	void setTouchMode(const ds::ui::TouchMode::Enum&);
	void createStatsView(sprite_id_t root_id);

	/// Read these values from settings and apply them
	void setupEngine(); /// calls all the below setup functions
	void setupLogger();
	void setupWorldSize();
	void setupSrcDstRects();
	void setupAutoSpan();
	void setupConsole();
	void setupWindowMode();
	void setupMouseHide();
	void setupFrameRate();
	void setupVerticalSync();
	void setupIdleTimeout();
	void setupMute();
	void setupResourceLocation();
	void setupRoots();
	void setupAutoRefresh();

	friend class cfg::SettingsEditor;
	std::vector<std::unique_ptr<EngineRoot>> mRoots;
	ds::App&								 mDsApp;
	ds::EngineSettings&						 mSettings;
	ds::cfg::SettingsEditor*				 mSettingsEditor;
	bool mHideMouseSaved = false;
	bool mAutoHideMouseSaved = false;

	bool									 mShowConsole;
	ds::ui::PangoFontService				 mPangoFontService;
	ds::ui::Tweenline						 mTweenline;
	/// A cache of all the resources in the system
	ResourceList mResources;
	ColorList	 mColors;
	FontList	 mFonts;
	RootList	 mRequestedRootList;
	UpdateParams mUpdateParams;
	DrawParams	 mDrawParams;
	float		 mLastTime;
	bool		 mIdling;
	bool		 mIdlingEnabled;
	float		 mLastTouchTime;

	/// Main tuio input
	std::shared_ptr<ds::ui::TuioInput> mTuioInput;
	/// Additional tuio inputs if configured
	std::vector<std::shared_ptr<ds::ui::TuioInput>> mTuioInputs;

	/// Clients that will get update() called automatically at the start
	/// of each update cycle
	AutoUpdateList mAutoUpdateServer;
	AutoUpdateList mAutoUpdateClient;
	/// Quick hack to get any ol' client participating in draw
	AutoDrawService* mAutoDraw;

	AutoRefresh mAutoRefresh;

	ds::ui::TouchTranslator						mTouchTranslator;
	std::mutex									mTouchMutex;
	ds::EngineTouchQueue<ds::ui::TouchEvent>	mTouchBeginEvents;
	ds::EngineTouchQueue<ds::ui::TouchEvent>	mTouchMovedEvents;
	ds::EngineTouchQueue<ds::ui::TouchEvent>	mTouchEndedEvents;
	typedef std::pair<ci::app::MouseEvent, int> MousePair;
	ds::EngineTouchQueue<MousePair>				mMouseBeginEvents;
	ds::EngineTouchQueue<MousePair>				mMouseMovedEvents;
	ds::EngineTouchQueue<MousePair>				mMouseEndedEvents;
	/// Only used if the settings file has "tuio:receive_objects" set to true
	ds::EngineTouchQueue<TuioObject> mTuioObjectsBegin;
	ds::EngineTouchQueue<TuioObject> mTuioObjectsMoved;
	ds::EngineTouchQueue<TuioObject> mTuioObjectsEnded;

	bool								  mRotateTouchesDefault;
	bool								  mAutoHideMouse;
	bool								  mHideMouse;
	ci::Color8u							  mUniqueColor;
	int									  mCachedWindowW, mCachedWindowH;
	ci::app::WindowRef					  mCinderWindow;
	std::shared_ptr<ui::LoadImageService> mLoadImageService;

	/// Channels. A channel is simply a notifier, with an optional description.
	class Channel {
	  public:
		Channel();
		Channel(const std::string& description);

		ds::EventNotifier mNotifier;
		std::string		  mDescription;
	};
	std::unordered_map<std::string, Channel> mChannels;

	float mAverageFps;

	/// For listening to settings changes and applying them
	void			onAppEvent(const ds::Event&);
	ds::EventClient mEventClient;
};

} // namespace ds
