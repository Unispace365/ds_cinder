#include "ds/app/engine_server.h"

namespace ds {

/**
 * \class ds::EngineServer
 */
EngineServer::EngineServer(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
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
