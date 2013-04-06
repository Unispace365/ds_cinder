#pragma once
#ifndef DS_APP_APP_H_
#define DS_APP_APP_H_

#include <cinder/app/AppBasic.h>
#include "ds/app/engine.h"
#include "ds/app/engine_settings.h"

namespace ds {
class Environment;

/**
 * \class ds::App
 * Handle the main app setup.
 */
class App : public cinder::app::AppBasic {
  public:
		// Apps can provide a list of root sprites (Engine::CAMERA_ORTHO or Engine::CAMERA_PERSP), which
		// can be accessed by index. If none supplied, you get 1 orthogonal.
    App(const std::vector<int>* roots = nullptr, const std::string &basePath = "");
    ~App();

    Engine                      &getEngine();
    ui::SpriteEngine            &getSpriteEngine();
    ui::Sprite                  &getRootSprite();

    virtual void                mouseDown( MouseEvent event );	
    virtual void                mouseMove( MouseEvent event );
    virtual void                mouseDrag( MouseEvent event );
    virtual void                mouseUp( MouseEvent event );	
    virtual void                touchesBegan( TouchEvent event );
    virtual void                touchesMoved( TouchEvent event );
    virtual void                touchesEnded( TouchEvent event );
    virtual void                keyDown( KeyEvent event );
    virtual void                keyUp( KeyEvent event );
    virtual void                prepareSettings(Settings*);
    virtual void				        setup();
    // This is where client applications would setup the initial UI.
    virtual void                setupServer()     { }
    virtual void	    			    update();
    virtual void	    			    draw();
    virtual void                quit();
    virtual void                shutdown();

	void						enableCommonKeystrokes(bool q = true, bool esc = true);

  protected:
    class Initializer { public: Initializer(const std::string&); };
    Initializer                 mInitializer;

    ds::EngineSettings          mEngineSettings;
    ds::Engine&                 mEngine;

  private:
    typedef ci::app::AppBasic   inherited;

    friend class Environment;
    static const std::string&   envAppPath();

    bool                        mCtrlDown;
    bool                        mSecondMouseDown;
	bool						mQKeyEnabled;
	bool						mEscKeyEnabled;

};

} // namespace ds

#endif // DS_APP_ENGINE_H_
