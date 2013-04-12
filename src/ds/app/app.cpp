#include "ds/app/app.h"

#include "ds/app/engine_client.h"
#include "ds/app/engine_clientserver.h"
#include "ds/app/engine_server.h"
#include "ds/debug/console.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
// For installing the sprite types
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/text.h"

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(ds::App&, const ds::cfg::Settings&, const std::vector<int>* roots);

namespace {
std::string           APP_PATH;
#ifdef _DEBUG
ds::Console		        GLOBAL_CONSOLE;
#endif
}

namespace ds {

/**
 * \class ds::App
 */
App::App(const std::vector<int>* roots)
	: mInitializer(getAppPath().generic_string())
	, mEngineSettings()
	, mEngine(new_engine(*this, mEngineSettings, roots))
	, mCtrlDown(false)
	, mSecondMouseDown(false)
	, mQKeyEnabled(false)
	, mEscKeyEnabled(false)
{
  // Initialize each sprite type with a unique blob handler for network communication.
  mEngine.installSprite([](ds::BlobRegistry& r){ds::ui::Sprite::installAsServer(r);},
                        [](ds::BlobRegistry& r){ds::ui::Sprite::installAsClient(r);});
  mEngine.installSprite([](ds::BlobRegistry& r){ds::ui::Image::installAsServer(r);},
                        [](ds::BlobRegistry& r){ds::ui::Image::installAsClient(r);});
  mEngine.installSprite([](ds::BlobRegistry& r){ds::ui::Text::installAsServer(r);},
                        [](ds::BlobRegistry& r){ds::ui::Text::installAsClient(r);});
}

App::~App()
{
  delete &(mEngine);
  ds::getLogger().shutDown();
  DS_DBG_CODE(GLOBAL_CONSOLE.destroy());
}

void App::prepareSettings(Settings *settings)
{
  inherited::prepareSettings(settings);

  if (settings) {
    mEngine.prepareSettings(*settings);

    // set in the engine.xml
    ci::Vec3f pos = mEngineSettings.getPoint("window_pos", 0, ci::Vec3f(0.0f, 0.0f, 0.0f));
    settings->setWindowPos(static_cast<unsigned>(pos.x), static_cast<unsigned>(pos.y));
  }
}

void App::setup()
{
  inherited::setup();

  mEngine.setup(*this);
  mEngine.setupTuio(*this);
}

void App::update()
{
  if (mEngine.hideMouse())
    hideCursor();
  mEngine.update();
}

void App::draw()
{
  mEngine.draw();
}

void App::mouseDown( MouseEvent event )
{
  if (mCtrlDown) {
    if (!mSecondMouseDown) {
      mEngine.mouseTouchBegin(event, 2);
      mSecondMouseDown = true;
    } else {
      mEngine.mouseTouchEnded(event, 2);
      mSecondMouseDown = false;
    }
  } else {
    mEngine.mouseTouchBegin(event, 1);
  }
}

void App::mouseMove( MouseEvent event )
{
}

void App::mouseDrag( MouseEvent event )
{
  mEngine.mouseTouchMoved(event, 1);
}

void App::mouseUp( MouseEvent event )
{
  mEngine.mouseTouchEnded(event, 1);
}

void App::touchesBegan( TouchEvent event )
{
  mEngine.touchesBegin(event);
}

void App::touchesMoved( TouchEvent event )
{
  mEngine.touchesMoved(event);
}

void App::touchesEnded( TouchEvent event )
{
  mEngine.touchesEnded(event);
}

const std::string& App::envAppPath()
{
  return APP_PATH;
}

void App::keyDown( KeyEvent event )
{
  if ( ( mEscKeyEnabled && event.getCode() == KeyEvent::KEY_ESCAPE ) || ( mQKeyEnabled && event.getCode() == KeyEvent::KEY_q ) )
    std::exit(0);
  if ( event.getCode() == KeyEvent::KEY_LCTRL || event.getCode() == KeyEvent::KEY_RCTRL )
    mCtrlDown = true;
}

void App::keyUp( KeyEvent event )
{
  if ( event.getCode() == KeyEvent::KEY_LCTRL || event.getCode() == KeyEvent::KEY_RCTRL )
    mCtrlDown = false;
}

void App::enableCommonKeystrokes( bool q /*= true*/, bool esc /*= true*/ ){
  if (q)
	mQKeyEnabled = true;
  if (esc)
    mEscKeyEnabled = true;
}

void App::quit()
{
  ci::app::AppBasic::quit();
}

void App::shutdown()
{
  mEngine.getRootSprite().clearChildren();
  ds::ui::clearFontCache();
  ci::app::AppBasic::shutdown();
}


/**
 * \class ds::App::Initializer
 */
ds::App::Initializer::Initializer(const std::string& appPath)
{
  DS_DBG_CODE(GLOBAL_CONSOLE.create());
  APP_PATH = appPath;
}

} // namespace ds

static ds::Engine&    new_engine(ds::App& app, const ds::cfg::Settings& settings, const std::vector<int>* roots)
{
  if (settings.getText("platform:architecture", 0, "") == "client") return *(new ds::EngineClient(app, settings, roots));
  if (settings.getText("platform:architecture", 0, "") == "server") return *(new ds::EngineServer(app, settings, roots));
  return *(new ds::EngineClientServer(app, settings, roots));
}
