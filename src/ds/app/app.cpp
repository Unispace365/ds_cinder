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

static ds::Engine&    new_engine(const ds::cfg::Settings& settings)
{
  return *(new ds::EngineClientServer(settings));
}
