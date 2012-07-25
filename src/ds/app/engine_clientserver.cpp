#include "ds/app/engine_clientserver.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 */
EngineClientServer::EngineClientServer(ds::App& app, const ds::cfg::Settings& settings)
    : inherited(app, settings)
    , mLoadImageService(mLoadImageThread)
{
}

EngineClientServer::~EngineClientServer()
{
  // It's important to clean up the sprites before the services go away
  mRootSprite.clearChildren();
}

void EngineClientServer::installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
                                       const std::function<void(ds::BlobRegistry&)>& asClient)
{
  // I don't have network communication so I don't need to handle blob.
}

void EngineClientServer::setup(ds::App& app)
{
  inherited::setup(app);

  mLoadImageThread.start(true);

  app.setupServer();
}

void EngineClientServer::setupTuio(ds::App& a)
{
  tuio::Client &tuioClient = getTuioClient();
  tuioClient.registerTouches(&a);
  tuioClient.connect();
}

void EngineClientServer::update()
{
  mWorkManager.update();
  updateServer();
}

void EngineClientServer::draw()
{
  drawClient();
}

} // namespace ds
