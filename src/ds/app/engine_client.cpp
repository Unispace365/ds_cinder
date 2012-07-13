#include "ds/app/engine_client.h"

namespace ds {

/**
 * \class ds::EngineClient
 */
EngineClient::EngineClient(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
}

void EngineClient::setup()
{
  inherited::setup();

  mLoadImageThread.start(true);
}

void EngineClient::setupTuio(ds::App&)
{
}

void EngineClient::update()
{
  updateClient();
}

void EngineClient::draw()
{
  drawClient();
}

} // namespace ds
