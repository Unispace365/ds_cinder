#include "ds/app/engine_server.h"

#include "ds/app/app.h"
#include "ds/app/blob_reader.h"

namespace ds {

namespace {
char              HEADER_BLOB = 0;
char              COMMAND_BLOB = 0;
}

/**
 * \class ds::EngineServer
 */
EngineServer::EngineServer(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
  // NOTE:  Must be EXACTLY the same items as in EngineClient
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});
}

void EngineServer::installSprite( const std::function<void(ds::BlobRegistry&)>& asServer,
                                  const std::function<void(ds::BlobRegistry&)>& asClient)
{
  if (asServer) asServer(mBlobRegistry);
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

  ui::Sprite                 &root = getRootSprite();
  if (root.isDirty()) {
    mSendBuffer.clear();
    root.writeTo(mSendBuffer);
  }
}

void EngineServer::draw()
{
  drawServer();
}

void EngineServer::receiveHeader(ds::DataBuffer& data)
{
}

void EngineServer::receiveCommand(ds::DataBuffer& data)
{
}

} // namespace ds
