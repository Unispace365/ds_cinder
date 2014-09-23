#pragma once
#ifndef DS_APP_APP_H_
#define DS_APP_APP_H_

#include <cinder/app/AppBasic.h>
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_settings.h"

namespace ds {
class Environment;
class TuioObject;

/**
 * \class ds::App
 * Handle the main app setup.
 */
class App : public ci::app::AppBasic {
public:
	// This is used for external projects to perform some initialization
	// on app startup time. It's intended to be called by clients from a
	// static initializer.
	// Note that throwing an exception in the function will exit the app.
	static void AddStartup(const std::function<void(ds::Engine&)>&);

	// Apps can provide a list of root sprites by chaining commands to a RootList.
	// For example, if you want a single perspective root, do this:
	// App(ds::RootList().persp())
	// See RootList class for full use. By default, you get a single
	// orthogonal root.
	App(const RootList& = RootList());
	~App();

	virtual void				mouseDown(ci::app::MouseEvent event);
	virtual void				mouseMove(ci::app::MouseEvent event);
	virtual void				mouseDrag(ci::app::MouseEvent event);
	virtual void				mouseUp(ci::app::MouseEvent event);

	// These are called from the boost thread
	// These events are sent to the engine to be queued for the next update
	virtual void				touchesBegan(ci::app::TouchEvent event);
	virtual void				touchesMoved(ci::app::TouchEvent event);
	virtual void				touchesEnded(ci::app::TouchEvent event);

	// These are safe to override
	virtual void				onTouchesBegan(ci::app::TouchEvent event){};
	virtual void				onTouchesMoved(ci::app::TouchEvent event){};
	virtual void				onTouchesEnded(ci::app::TouchEvent event){};

	// To receive TUIO Objects, the engine must have this setting:
	//	<text name="tuio:receive_objects" value="true" />
	virtual void				tuioObjectBegan( const TuioObject& );
	virtual void				tuioObjectMoved( const TuioObject& );
	virtual void				tuioObjectEnded( const TuioObject& );
	virtual void				keyDown(ci::app::KeyEvent event);
	virtual void				keyUp(ci::app::KeyEvent event);
	virtual void				prepareSettings( Settings* );
	virtual void				setup();
	// This is where client applications would setup the initial UI.
	virtual void				setupServer() { }
	virtual void				update();
	virtual void				draw();
	virtual void				quit();
	virtual void				shutdown();

	void						showConsole();

	void						enableCommonKeystrokes(bool q = true, bool esc = true);

protected:
	class Initializer { public: Initializer(const std::string&); };
	Initializer					mInitializer;

	bool						mShowConsole;
	ds::EngineSettings			mEngineSettings;
	ds::EngineData				mEngineData;
	ds::Engine&					mEngine;

private:
	typedef ci::app::AppBasic   inherited;

	friend class Environment;
	// Path to the executable (which realistically we never want)
	static const std::string&   envAppPath();
	// Path to the folder that contains the "data" folder
	// (but not including "data", you still need to add that
	// if it's what you want
	static const std::string&   envAppDataPath();

	bool						mCtrlDown;
	bool						mSecondMouseDown;
	bool						mQKeyEnabled;
	bool						mEscKeyEnabled;
	// When enabled, the arrow keys will move the camera.
	const float					mArrowKeyCameraStep;
	const bool					mArrowKeyCameraControl;
};

} // namespace ds

#endif // DS_APP_ENGINE_H_