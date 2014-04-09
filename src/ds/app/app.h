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

	// Apps can provide a list of root sprites (Engine::CAMERA_ORTHO or Engine::CAMERA_PERSP), which
	// can be accessed by index. If none supplied, you get 1 orthogonal.
	App(const std::vector<int>* roots = nullptr);
	~App();

	virtual void				mouseDown( MouseEvent event );	
	virtual void				mouseMove( MouseEvent event );
	virtual void				mouseDrag( MouseEvent event );
	virtual void				mouseUp(   MouseEvent event );	

	// These are called from the boost thread
	// These events are sent to the engine to be queued for the next update
	virtual void				touchesBegan( TouchEvent event );
	virtual void				touchesMoved( TouchEvent event );
	virtual void				touchesEnded( TouchEvent event );

	// These are safe to override
	virtual void				onTouchesBegan( TouchEvent event ){};
	virtual void				onTouchesMoved( TouchEvent event ){};
	virtual void				onTouchesEnded( TouchEvent event ){};

	// To receive TUIO Objects, the engine must have this setting:
	//	<text name="tuio:receive_objects" value="true" />
	virtual void				tuioObjectBegan( const TuioObject& );
	virtual void				tuioObjectMoved( const TuioObject& );
	virtual void				tuioObjectEnded( const TuioObject& );
	virtual void				keyDown( KeyEvent event );
	virtual void				keyUp( KeyEvent event );
	virtual void				prepareSettings( Settings* );
	virtual void				setup();
	// This is where client applications would setup the initial UI.
	virtual void				setupServer() { }
	virtual void				update();
	virtual void				draw();
	virtual void				quit();
	virtual void				shutdown();

	void						enableCommonKeystrokes(bool q = true, bool esc = true);

protected:
	class Initializer { public: Initializer(const std::string&); };
	Initializer					mInitializer;

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