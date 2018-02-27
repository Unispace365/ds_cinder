#pragma once
#ifndef DS_APP_APP_H_
#define DS_APP_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include "ds/app/app_defs.h"
#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_settings.h"
#include "ds/ui/touch/touch_debug.h"
#include "ds/ui/touch/touch_event.h"

namespace ds {
class Environment;
class TuioObject;
class Engine;

/**
 * \class ds::EngineSettingsPreloader
 * Load engine settings first, then setup ci::app settings accordingly,
 * before Cinder's App instantiation and Window creation
 */
class EngineSettingsPreloader {
public:
	EngineSettingsPreloader( ci::app::AppBase::Settings* settings );

protected:
	virtual void				earlyPrepareAppSettings( ci::app::AppBase::Settings* settings );
	class Initializer {
	public:
								Initializer();
	};
	Initializer					mInitializer;

	ds::EngineSettings			mEngineSettings;
};

/**
 * \class ds::App
 * Handle the main app setup.
 */
class App : public EngineSettingsPreloader, public cinder::app::App {
private:
	const bool			mEnvironmentInitialized;

public:
	// This is used for external projects to perform some initialization
	// on app startup time. It's intended to be called by clients from a
	// static initializer.
	// Note that throwing an exception in the function will exit the app.
	static void AddStartup(const std::function<void(ds::Engine&)>&);

	// Called just before the main app setupServer() virtual function
	static void AddServerSetup(const std::function<void(ds::Engine&)>&);

	// Apps can provide a list of root sprites by chaining commands to a RootList.
	// For example, if you want a single perspective root, do this:
	// App(ds::RootList().persp())
	// See RootList class for full use. By default, you get a single
	// orthogonal root.
	App(const RootList& = RootList());
	~App();

	virtual void				mouseDown( ci::app::MouseEvent event );	
	virtual void				mouseMove( ci::app::MouseEvent event );
	virtual void				mouseDrag( ci::app::MouseEvent event );
	virtual void				mouseUp(   ci::app::MouseEvent event );	

	// These are called from the boost thread
	// These events are sent to the engine to be queued for the next update
	// NOTE: do not call these from your client app. use the inject functions on SpriteEngine to put touch events into the system
	virtual void				touchesBegan(ci::app::TouchEvent event) final;
	virtual void				touchesMoved(ci::app::TouchEvent event) final;
	virtual void				touchesEnded(ci::app::TouchEvent event) final;

	// These are safe to override
	virtual void				onTouchesBegan(ds::ui::TouchEvent event){};
	virtual void				onTouchesMoved(ds::ui::TouchEvent event){};
	virtual void				onTouchesEnded(ds::ui::TouchEvent event){};

	/// These are here to throw a compiler error on projects with the legacy events. These are no longer called, so use the above ds::ui::TouchEvent callbacks
	/// DEPRECATED
	virtual void				onTouchesBegan(ci::app::TouchEvent event) final {};
	virtual void				onTouchesMoved(ci::app::TouchEvent event) final {};
	virtual void				onTouchesEnded(ci::app::TouchEvent event) final {};

	// To receive TUIO Objects, the engine must have this setting:
	//	<text name="touch:tuio:receive_objects" value="true" />
	virtual void				tuioObjectBegan(const TuioObject&);
	virtual void				tuioObjectMoved(const TuioObject&);
	virtual void				tuioObjectEnded(const TuioObject&);

	/// Key events coming from the base Cinder App class
	virtual void				keyDown(ci::app::KeyEvent event) final;
	virtual void				keyUp(ci::app::KeyEvent event) final;

	/// If false, will send all keys to the client's app class (disables escape-to-quit, "s" for status pane, etc)
	/// If true, the default, will parse keys first and send any remaining key presses to the client app (enables the normal keys like escape-to-quit, "f" for fullscreen)
	void						setAppKeysEnabled(const bool enabled){ mAppKeysEnabled = enabled; }

	/// Override these to get key notifications
	/// The app may not pass some keys in some circumstances (like there's a registered soft keyboard)
	virtual void				onKeyDown(ci::app::KeyEvent event){};
	virtual void				onKeyUp(ci::app::KeyEvent event){};

	virtual void				prepareSettings( ci::app::AppBase::Settings* );
	void						loadAppSettings();
	virtual void				setup();
	void						resetupServer();
	void						preServerSetup();

	// This is where client applications would setup the initial UI.
	virtual void				setupServer() {}
	virtual void				update();
	virtual void				draw();
	virtual void				quit();
	virtual void				shutdown();

	// Triggered by F8 key, saves a transparent png on the desktop
	void						saveTransparentScreenshot();

protected:
	ds::EngineData				mEngineData;
	ds::Engine&					mEngine;

private:
	typedef ci::app::App   inherited;

	friend class Environment;
	// Path to the folder that contains the "data" folder
	// (but not including "data", you still need to add that
	// if it's what you want
	static const std::string&   envAppDataPath();
	bool						mCtrlDown;
	ds::ui::TouchDebug			mTouchDebug;
	bool						mAppKeysEnabled;
	bool						mMouseHidden;
	Poco::Timestamp::TimeVal	mMouseMoveTime;

	// When enabled, the arrow keys will move the camera.
	const float					mArrowKeyCameraStep;
	const bool					mArrowKeyCameraControl;
};

} // namespace ds

#endif // DS_APP_ENGINE_H_
