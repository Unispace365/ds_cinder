#include "ds/app/engine_server.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineServer
 */
EngineServer::EngineServer(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
}

void EngineServer::setupTuio(ds::App& a)
{
  tuio::Client &tuioClient = getTuioClient();
  tuioClient.registerTouches(&a);
  tuioClient.connect();
}

void EngineServer::update()
{
  updateServer();
}

void EngineServer::draw()
{
  drawServer();
}

} // namespace ds
