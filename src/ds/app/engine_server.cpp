#include "ds/app/engine_server.h"

#include "ds/app/app.h"
#include "snappy.h"
#include "ds/util/string_util.h"

namespace ds {

/**
 * \class ds::EngineServer
 */
EngineServer::EngineServer(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
    , mConnection(NumberOfNetworkThreads)
{
  mConnection.initialize(true, settings.getText("server:ip", 0, "239.255.20.20"), ds::value_to_string(settings.getInt("server:send_port", 0, 8000)));
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

} // namespace ds
