#pragma once
#ifndef DS_APP_APP_H_
#define DS_APP_APP_H_

#include <cinder/app/App.h>

#include "ds/app/app_defs.h"
#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_settings.h"
#include "ds/content/service/bridge_sync_service.h"
#include "ds/content/service/sync_service.h"
#include "ds/debug/key_manager.h"
#include "ds/ui/touch/touch_debug.h"
#include "ds/ui/touch/touch_event.h"

namespace ds {
class Environment;
class TuioObject;
class Engine;

/**
 * \class EngineSettingsPreloader
 * Load engine settings first, then setup ci::app settings accordingly,
 * before Cinder's App instantiation and Window creation
 */
class EngineSettingsPreloader {
  public:
	EngineSettingsPreloader(ci::app::AppBase::Settings* settings);

  protected:
	virtual void earlyPrepareAppSettings(ci::app::AppBase::Settings* settings);
	class Initializer {
	  public:
		Initializer();
	};
	Initializer mInitializer;

	EngineSettings mEngineSettings;
};

/**
 * \class App
 * Handle the main app setup.
 */
class App : public EngineSettingsPreloader, public cinder::app::App {
  private:
	const bool mEnvironmentInitialized;

  public:
	/// This is used for external projects to perform some initialization
	/// on app startup time. It's intended to be called by clients from a
	/// static initializer.
	/// Note that throwing an exception in the function will exit the app.
	static void AddStartup(const std::function<void(Engine&)>&);

	static void AddStartup(std::string name, const std::function<void(Engine&)>& fn);

	/// Called just before the main app setupServer() virtual function
	static void AddServerSetup(const std::function<void(Engine&)>&);

	/// Apps can provide a list of root sprites by chaining commands to a RootList.
	/// For example, if you want a single perspective root, do this:
	/// App(ds::RootList().persp())
	/// See RootList class for full use. By default, you get a single
	/// orthogonal root.
	App(const RootList& = RootList());
	~App() override;

	void mouseDown(ci::app::MouseEvent event) override;
	void mouseMove(ci::app::MouseEvent event) override;
	void mouseDrag(ci::app::MouseEvent event) override;
	void mouseUp(ci::app::MouseEvent event) override;

	/// These are called from the boost thread
	/// These events are sent to the engine to be queued for the next update
	/// NOTE: do not call these from your client app. use the inject functions on SpriteEngine to put touch events into
	/// the system
	void touchesBegan(ci::app::TouchEvent event) final;
	void touchesMoved(ci::app::TouchEvent event) final;
	void touchesEnded(ci::app::TouchEvent event) final;

	/// These are safe to override
	virtual void onTouchesBegan(ui::TouchEvent event){}
	virtual void onTouchesMoved(ui::TouchEvent event){}
	virtual void onTouchesEnded(ui::TouchEvent event){}

	/// These are here to throw a compiler error on projects with the legacy events. These are no longer called, so use
	/// the above ds::ui::TouchEvent callbacks DEPRECATED
	[[deprecated]] virtual void onTouchesBegan(ci::app::TouchEvent event) final{}
	[[deprecated]] virtual void onTouchesMoved(ci::app::TouchEvent event) final{}
	[[deprecated]] virtual void onTouchesEnded(ci::app::TouchEvent event) final{}

	/// To receive TUIO Objects, the engine must have this setting:
	///	\code <text name="touch:tuio:receive_objects" value="true" /> \endcode
	virtual void tuioObjectBegan(const TuioObject&);
	virtual void tuioObjectMoved(const TuioObject&);
	virtual void tuioObjectEnded(const TuioObject&);

	/// Key events coming from the base Cinder App class
	void keyDown(ci::app::KeyEvent event) final;
	void keyUp(ci::app::KeyEvent event) final;

	/// If false, will send all keys to the client's app class (disables escape-to-quit, "s" for status pane, etc)
	/// If true, the default, will parse keys first and send any remaining key presses to the client app (enables the
	/// normal keys like escape-to-quit, "f" for fullscreen)
	void setAppKeysEnabled(const bool enabled) { mAppKeysEnabled = enabled; }

	/// Override these to get key notifications
	/// The app may not pass some keys in some circumstances (like there's a registered soft keyboard)
	virtual void onKeyDown(ci::app::KeyEvent event){}
	virtual void onKeyUp(ci::app::KeyEvent event){}

	virtual void prepareSettings(AppBase::Settings*);
	void         loadAppSettings() const;
	void         setup() override;
	void         resetupServer();
	void         preServerSetup() const;

	/// This is where client applications would setup the initial UI.
	virtual void setupServer() {}
	void         update() override;
	void         draw() override;
	void         quit() override;
	virtual void shutdown();

	/// Triggered by F8 key, saves a transparent png on the desktop
	void saveTransparentScreenshot();

	/// Kills RoC, dsnode, then this app
	void killSupportingApps();

	/// Logs all sprites to disk for deep debuggin
	void writeSpriteHierarchy();

	/// Show sprites that are enabled
	void debugEnabledSprites() const;

	/// Launch downsync service
	void launchSyncService();

	/// Launch downsync service
	void launchBridgeSyncService();

	/// Register a function to be called when a key is pressed (with optional modifier keys)
	/// The Key codes can be found in ci::app::KeyEvent
	/// This will be displayed with the help debug
	void registerKeyPress(const std::string& name, std::function<void()> func, const int keyCode,
						  const bool shiftDown = false, const bool ctrlDown = false, const bool altDown = false);

	/// Get the instance of the key manager
	keys::KeyManager& getKeyManager() { return mKeyManager; }

  protected:
	EngineData mEngineData;
	Engine&    mEngine;

  private:
	using Inherited = ci::app::App;

	friend class Environment;
	/// Path to the folder that contains the "data" folder
	/// (but not including "data", you still need to add that
	/// if it's what you want
	static const std::string& envAppDataPath();
	void                      setupKeyPresses();
	void                      resetupServerOnDisplayChange();
	void                      resetTheWindow();
	bool                      mSetupOnDisplayChange;
	keys::KeyManager          mKeyManager;
	ui::TouchDebug            mTouchDebug;
	bool                      mAppKeysEnabled;
	bool                      mMouseHidden;
	bool                      mNeedsDockingWindow = true;
	Poco::Timestamp::TimeVal  mMouseMoveTime;

	/// When enabled, the arrow keys will move the camera.
	const float mArrowKeyCameraStep;
	const bool	mArrowKeyCameraControl;

	content::SyncService*       mSyncService       = nullptr;
	content::BridgeSyncService* mBridgeSyncService = nullptr;
};

} // namespace ds

#endif // DS_APP_ENGINE_H_
