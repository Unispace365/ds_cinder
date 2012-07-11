#include "ds/app/app.h"

#include "ds/app/engine_clientserver.h"

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(const ds::cfg::Settings&);

namespace ds {

/**
 * \class ds::App
 */
App::App()
  : mEngineSettings(getAppPath().generic_string())
  , mEngine(new_engine(mEngineSettings))
{
}

App::~App()
{
  delete &(mEngine);
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

} // namespace ds

static ds::Engine&    new_engine(const ds::cfg::Settings& settings)
{
  return *(new ds::EngineClientServer(settings));
}
