#include "ds/app/engine_server.h"

#include "ds/app/app.h"
#include "ds/app/blob_reader.h"
#include "ds/debug/logger.h"
#include "snappy.h"
#include "ds/util/string_util.h"

namespace ds {

namespace {
char              HEADER_BLOB = 0;
char              COMMAND_BLOB = 0;

const char        TERMINATOR = 0;
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

  try {
    mConnection.initialize(true, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
  } catch (std::exception &e) {
    DS_LOG_ERROR_M("EngineServer() initializing 0MQ: " << e.what(), ds::ENGINE_LOG);
  }
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

  // Always send the header
  mSendBuffer.clear();
  mSendBuffer.add(HEADER_BLOB);
  mSendBuffer.add(ds::TERMINATOR_CHAR);

  ui::Sprite                 &root = getRootSprite();
  if (root.isDirty()) {
    root.writeTo(mSendBuffer);
  }

  if (mConnection.initialized()) {
    if (mSendBuffer.size() > 0) {
      int size = mSendBuffer.size();
      mRawDataBuffer.setSize(size);
      mSendBuffer.readRaw(mRawDataBuffer.data(), size);
      snappy::Compress(mRawDataBuffer.data(), size, &mCompressionBufferWrite);
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
