#pragma once

#include <memory>
#include <unordered_map>

#include <cinder/Camera.h>
#include <cinder/app/App.h>

#include "ds/app/app_defs.h"
#include "ds/app/auto_update_list.h"
#include "ds/app/blob_registry.h"
#include "ds/app/engine/engine_settings.h"
#include "ds/app/engine/engine_touch_queue.h"
#include "ds/app/event_client.h"
#include "ds/app/event_notifier.h"
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
extern const BitMask ENGINE_LOG;
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

	~Engine() override;

	virtual void update() = 0;
	virtual void draw()	  = 0;

	void			notifyOnChannel(const Event& event, const std::string& channel, bool defaultAlso = false) override;
	void			notifyOnChannels(const Event& event, std::initializer_list<std::string> channels,
									 bool defaultAlso = false) override;
	EventNotifier&	getChannel(const std::string&) override;
	void			addChannel(const std::string& name, const std::string& description);
	AutoUpdateList& getAutoUpdateList(int = AutoUpdateType::SERVER) override;
	ui::PangoFontService& getPangoFontService() override { return mPangoFontService; }
	ui::LoadImageService& getLoadImageService() override { return *mLoadImageService; }
	ui::Tweenline&		  getTweenline() override { return mTweenline; }

	/// I take ownership of any services added to me.
	void addService(const std::string&, EngineService&) const;

	/// Convenience to load a setting file into the mEngineCfg settings.
	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the leaf path of the settings file (i.e. "data.xml").
	/// It will be loaded from all appropriate locations.
	void loadSettings(const std::string& name, const std::string& filename) override;

	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the leaf path of the settings file (i.e. "data.xml").
	/// It will be saved ONLY in the user settings location.
	void saveSettings(const std::string& name, const std::string& filename) const;

	/// Convenience to append a setting file into the existing mEngineCfg settings.
	/// \param name is the name that the system will use to refer to the settings.
	/// \param filename is the FULL path of the settings file (i.e. "C:\projects\settings\data.xml").
	/// It will NOT be loaded from all appropriate locations.
	void appendSettings(const std::string& name, const std::string& filename) const;

	/// Convenience to load a text cfg file into a collection of cfg objects.
	/// \param filename is the leaf path of the settings file (i.e. "text.xml").
	/// It will be loaded from all appropriate locations.
	void loadTextCfg(const std::string& filename);

	const EngineData& getEngineData() const { return mData; }
	/// only valid after setup() is called
	size_t		getRootCount() const;
	ui::Sprite& getRootSprite(size_t index = 0) const;
	/// Returns nullptr if the root sprite doesn't exist
	ui::Sprite* getRootSpritePtr(size_t index = 0) const;
	/// Access to the configuration settings that created a root. Allows you to inspect pick style, debug drawing,
	/// perspective, etc
	const RootList::Root& getRootBuilder(size_t index = 0) const;

	void prepareSettings(ci::app::AppBase::Settings&) const;
	void reloadSettings();
	void toggleSettingsEditor(const std::string& name = "");
	void showSettingsEditor(const std::string& name = "");
	void hideSettingsEditor();
	bool isShowingSettingsEditor() const;

	/// Called in app setup; loads settings files and what not.
	virtual void setup(App&);
	void		 setupTouch(App&);
	void		 startTuio(App&);
	void		 stopTuio() const;

	/// Returns whether idle events and checks are enabled.
	bool isIdlingEnabled() const override { return mIdlingEnabled; }

	/// Sets whether idle events and checks are enabled.
	void enableIdling(bool enable) override { mIdlingEnabled = enable; }

	/// It's been enough time since the last input and is in idle mode
	bool isIdling() override;

	/// Checks if it's been enough time since the last input to go into idle. Will take effect if it's been enough time
	void checkIdle();

	/// Starts idle mode right away, regardless of time
	void startIdling() override;

	/// Ends idle mode, regardless of input and starts the timeout again
	virtual void stopIdling() { resetIdleTimeout(); }

	/// Identical to stopIdling(), retained for backwards compatibility
	void resetIdleTimeout() override;

	/// Called during app construction, to register the sprites as blob handlers.
	virtual void installSprite(const std::function<void(BlobRegistry&)>& asServer,
							   const std::function<void(BlobRegistry&)>& asClient) = 0;

	sprite_id_t nextSpriteId() override;
	void		registerSprite(ui::Sprite&) override;
	void		unregisterSprite(ui::Sprite&) override;
	ui::Sprite* findSprite(sprite_id_t) override;
	void		spriteDeleted(sprite_id_t) override;
	ci::Color8u getUniqueColor() override;

	std::shared_ptr<ci::tuio::Receiver> getTuioClient(int tuioIndex = -1) const;
	void								touchesBegin(const ui::TouchEvent&);
	void								touchesMoved(const ui::TouchEvent&);
	void								touchesEnded(const ui::TouchEvent&);
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
	void injectTouchesBegin(const ui::TouchEvent&) override;
	void injectTouchesMoved(const ui::TouchEvent&) override;
	void injectTouchesEnded(const ui::TouchEvent&) override;

	void injectObjectsBegin(const TuioObject&) override;
	void injectObjectsMoved(const TuioObject&) override;
	void injectObjectsEnded(const TuioObject&) override;

	/// Register a tuio::Receiver to send TUIO objects events through the Engine.  Useful if your app needs
	/// additional tuio::Receiver object listeners beyond the single tuio::Receiver provided by the Engine.
	void registerForTuioObjects(const std::shared_ptr<ci::tuio::Receiver>&);

	/// Turns on Sprite's setRotateTouches when first created so you can enable rotated touches app-wide by default
	/// Sprites can still turn this off after creation
	bool getRotateTouchesDefault() override;

	ResourceList&	getResources() override;
	const FontList& getFonts() const override;
	FontList&		editFonts();

	const ColorList& getColors() const override;
	ColorList&		 getColors() override;
	ColorList&		 editColors();

	void				   markCameraDirty() const;
	PerspCameraParams	   getPerspectiveCamera(size_t index) const override;
	const ci::CameraPersp& getPerspectiveCameraRef(size_t index) const override;
	void				   setPerspectiveCamera(size_t index, const PerspCameraParams&) override;
	void				   setPerspectiveCameraRef(size_t index, const ci::CameraPersp&) override;

	float getOrthoFarPlane(size_t index) const override;
	float getOrthoNearPlane(size_t index) const override;
	void  setOrthoViewPlanes(size_t index, float nearPlane, float farPlane) override;

	/// Can be used by apps to stop services before exiting.
	/// This will happen automatically, but some apps might want
	/// to make sure everything is stopped before they go away.
	virtual void stopServices();

	void setHideMouse(bool doMouseHide);
	bool getHideMouse() const;
	bool getAutoHideMouse() const { return mAutoHideMouse; }

	ui::Sprite* getHit(const ci::vec3& point) override;

	ui::TouchManager& getTouchManager() { return mTouchManager; }
	void			  clearFingers(const std::vector<int>& fingers) override;

	void clearFingersForSprite(ui::Sprite* theSprite) override { mTouchManager.clearFingersForSprite(theSprite); }
	void setSpriteForFinger(const int fingerId, ui::Sprite* theSprite) override {
		mTouchManager.setSpriteForFinger(fingerId, theSprite);
	}

	ui::Sprite*	 getSpriteForFinger(const int fingerId) override { return mTouchManager.getSpriteForFinger(fingerId); }
	virtual bool shouldDiscardTouch(ci::vec2& p) { return mTouchManager.shouldDiscardTouch(p); }

	void setTouchSmoothing(bool doSmoothing);
	bool getTouchSmoothing();
	void setTouchSmoothFrames(int smoothFrames);

	/// Utility to change touch mode
	void nextTouchMode();

	/// Debugging aid to write out the sprites
	void writeSprites(std::ostream&) const;

	ci::app::WindowRef getWindow() override;

	void toggleConsole();
	void showConsole();
	void hideConsole();

	/// Should only be used by the app class to record the average fps.
	/// Allows for debug drawing of the fps
	void  setAverageFps(const float fps) { mAverageFps = fps; }
	float getAverageFps() const { return mAverageFps; }

	size_t getNumberOfSprites() const { return mSprites.size(); }

	/// -------------------------------------------------------------
	/// These functions are inlined, since they are called frequently
	/// -------------------------------------------------------------
	/// Returns the list of current roots
	const std::vector<std::unique_ptr<EngineRoot>>& getRoots() const { return mRoots; }
	const DrawParams&								getDrawParams() const { return mDrawParams; }
	AutoDrawService*								getAutoDrawService() const { return mAutoDraw; }

	/// This is for Clients to reconstruct roots when they re-connect with the server
	void clearRoots();

	/// For Clients to create roots when reconnecting to the server
	void createClientRoots(const std::vector<RootList::Root>& roots);


	/** Called from the destructor of all subclasses, so I can cleanup sprites before services go away.
	\param clearDebug If true, will clear all the children from the debug roots too.
	If false, leaves them alone (for instance, in client situations) */
	void clearAllSprites(bool clearDebug = true) const;

  protected:
	Engine(App&, EngineSettings&, EngineData&, const RootList&, int appMode);

	/// Conveniences for the subclasses
	void updateClient();
	void updateServer();
	void drawClient() const;
	void drawServer() const;

	/** When mouse events are ready to be handled by the touch manager.
		These are enforced virtual functions to be sure the engine handles mouse events.
		Servers will send directly to the touch manager, and clients can send back to the server */
	virtual void handleMouseTouchBegin(const ci::app::MouseEvent&, int id) = 0;
	virtual void handleMouseTouchMoved(const ci::app::MouseEvent&, int id) = 0;
	virtual void handleMouseTouchEnded(const ci::app::MouseEvent&, int id) = 0;


	ui::TouchManager mTouchManager;

	static const int NUMBER_OF_NETWORK_THREADS;

	BlobRegistry								 mBlobRegistry;
	std::unordered_map<sprite_id_t, ui::Sprite*> mSprites;
	int											 mTuioPort;

	ui::TouchMode::Enum mTouchMode;

  private:
	void setTouchMode(const ui::TouchMode::Enum&);
	void createStatsView(sprite_id_t rootId);

	/// Read these values from settings and apply them
	void setupEngine(); /// calls all the below setup functions
	void setupLogger() const;
	void setupWorldSize() const;
	void setupSrcDstRects() const;
	void setupAutoSpan() const;
	void setupConsole();
	void setupWindowMode() const;
	void setupMouseHide();
	void setupFrameRate() const;
	void setupVerticalSync() const;
	void setupIdleTimeout() const;
	void setupMute() const;
	void setupResourceLocation() const;
	void setupRoots();
	void setupAutoRefresh();

	friend class cfg::SettingsEditor;
	std::vector<std::unique_ptr<EngineRoot>> mRoots;
	App&									 mDsApp;
	EngineSettings&							 mSettings;
	cfg::SettingsEditor*					 mSettingsEditor;
	bool									 mHideMouseSaved	 = false;
	bool									 mAutoHideMouseSaved = false;

	bool				 mShowConsole;
	ui::PangoFontService mPangoFontService;
	ui::Tweenline		 mTweenline;
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
	std::shared_ptr<ui::TuioInput> mTuioInput;
	/// Additional tuio inputs if configured
	std::vector<std::shared_ptr<ui::TuioInput>> mTuioInputs;

	/// Clients that will get update() called automatically at the start
	/// of each update cycle
	AutoUpdateList mAutoUpdateServer;
	AutoUpdateList mAutoUpdateClient;
	/// Quick hack to get any ol' client participating in draw
	AutoDrawService* mAutoDraw;

	AutoRefresh mAutoRefresh;

	ui::TouchTranslator				 mTouchTranslator;
	std::mutex						 mTouchMutex;
	EngineTouchQueue<ui::TouchEvent> mTouchBeginEvents;
	EngineTouchQueue<ui::TouchEvent> mTouchMovedEvents;
	EngineTouchQueue<ui::TouchEvent> mTouchEndedEvents;

	using MousePair = std::pair<ci::app::MouseEvent, int>;
	EngineTouchQueue<MousePair> mMouseBeginEvents;
	EngineTouchQueue<MousePair> mMouseMovedEvents;
	EngineTouchQueue<MousePair> mMouseEndedEvents;

	/// Only used if the settings file has "tuio:receive_objects" set to true
	EngineTouchQueue<TuioObject> mTuioObjectsBegin;
	EngineTouchQueue<TuioObject> mTuioObjectsMoved;
	EngineTouchQueue<TuioObject> mTuioObjectsEnded;

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

		EventNotifier mNotifier;
		std::string	  mDescription;
	};
	std::unordered_map<std::string, Channel> mChannels;

	float mAverageFps;

	/// For listening to settings changes and applying them
	void		onAppEvent(const Event&);
	EventClient mEventClient;
};

} // namespace ds
