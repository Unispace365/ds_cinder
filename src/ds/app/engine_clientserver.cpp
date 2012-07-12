#include "ds/app/engine_clientserver.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 */
EngineClientServer::EngineClientServer(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
}

void EngineClientServer::setup()
{
  inherited::update();

  mLoadImageThread.start(true);
}

void EngineClientServer::update()
{
  mWorkManager.update();

  inherited::update();
}

} // namespace ds
