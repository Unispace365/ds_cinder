#include "ds/app/engine_server.h"

#include "ds/app/app.h"
#include "ds/app/blob_reader.h"
#include "snappy.h"
#include "ds/util/string_util.h"

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
    , mConnection(NumberOfNetworkThreads)
{
  // NOTE:  Must be EXACTLY the same items as in EngineClient
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});
  mConnection.initialize(true, settings.getText("server:ip", 0, "239.255.20.20"), ds::value_to_string(settings.getInt("server:send_port", 0, 8000)));
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
    //create header
    //
    root.writeTo(mSendBuffer);
  }

  if (mConnection.initialized()) {
    if (mSendBuffer.size() > 0) {
      int size = mSendBuffer.size();
      mRawDataBuffer.resize(size);
      mSendBuffer.readRaw(mRawDataBuffer.ptr(), size);
      snappy::Compress(mRawDataBuffer.constPtr(), size, &mCompressionBufferWrite);
      mConnection.sendMessage(mCompressionBufferWrite);
      mSendBuffer.clear();
    }

    if (mConnection.recvMessage(mCompressionBufferWrite)) {
      snappy::Uncompress(mCompressionBufferWrite.c_str(), mCompressionBufferWrite.size(), &mCompressionBufferRead);
    }
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
