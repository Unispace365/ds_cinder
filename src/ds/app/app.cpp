#include "ds/app/app.h"

#include "ds/app/engine_clientserver.h"
#include "ds/app/engine_server.h"
#include "ds/debug/console.h"
#include "ds/debug/debug_defines.h"

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(const ds::cfg::Settings&);

namespace {
#ifdef _DEBUG
ds::Console		GLOBAL_CONSOLE;
#endif
}

namespace ds {

/**
 * \class ds::App
 */
App::App()
  : mInitializer(getAppPath().generic_string())
  , mEngineSettings(getAppPath().generic_string())
  , mEngine(new_engine(mEngineSettings))
{
}

App::~App()
{
  delete &(mEngine);
  DS_DBG_CODE(GLOBAL_CONSOLE.destroy());
}

void App::prepareSettings(Settings *settings)
{
  inherited::prepareSettings(settings);

  mEngine.loadCinderSettings(settings);
}

void App::setup()
{
  inherited::setup();

  mEngine.setup();

  tuio::Client &tuioClient = mEngine.getTuioClient();
  tuioClient.registerTouches(this);
  tuioClient.connect();
}

void App::update()
{
  mEngine.update();
}

void App::draw()
{
  mEngine.draw();
}

void App::mouseDown( MouseEvent event )
{
  mEngine.mouseTouchBegin(event, 1);
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

/**
 * \class ds::App::Initializer
 */
ds::App::Initializer::Initializer(const std::string& appPath)
{
  DS_DBG_CODE(GLOBAL_CONSOLE.create());
}

} // namespace ds

static ds::Engine&    new_engine(const ds::cfg::Settings& settings)
{
  if (settings.getText("platform:architecture", 0, "") == "server") return *(new ds::EngineServer(settings));
  return *(new ds::EngineClientServer(settings));
}
