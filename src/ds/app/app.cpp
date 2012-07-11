#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::App
 */
App::App()
{
}

App::~App()
{
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

  for (auto it=getArgs().begin(), end=getArgs().end(); it != end; ++it) {
    std::cout << "ARG=" << (*it) << std::endl;
  }
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
