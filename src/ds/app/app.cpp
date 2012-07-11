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

} // namespace ds
