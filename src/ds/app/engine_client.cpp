#include "ds/app/engine_client.h"

#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"
#include "ds/util/string_util.h"
#include "snappy.h"

namespace ds {

namespace {
char              HEADER_BLOB = 0;
char              COMMAND_BLOB = 0;
}

/**
 * \class ds::EngineClient
 */
EngineClient::EngineClient(ds::App& app, const ds::cfg::Settings& settings)
    : inherited(app, settings)
    , mLoadImageService(mLoadImageThread)
    , mBlobReader(mReceiveBuffer, *this)
    , mConnection(NumberOfNetworkThreads)
{
  // NOTE:  Must be EXACTLY the same items as in EngineServer
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});

  try {
   mConnection.initialize(false, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
  } catch(std::exception &e) {
    DS_LOG_ERROR_M("EngineClient() initializing 0MQ: " << e.what(), ds::ENGINE_LOG);
  }
}

EngineClient::~EngineClient()
{
  // It's important to clean up the sprites before the services go away
  mRootSprite.clearChildren();
}

void EngineClient::installSprite( const std::function<void(ds::BlobRegistry&)>& asServer,
                                  const std::function<void(ds::BlobRegistry&)>& asClient)
{
  if (asClient) asClient(mBlobRegistry);
}

ds::sprite_id_t EngineClient::nextSpriteId()
{
  // Clients never generate sprite IDs, they are always assigned from a blob.
  return 0;
}

void EngineClient::setup(ds::App& app)
{
  inherited::setup(app);

  mLoadImageThread.start(true);
}

void EngineClient::setupTuio(ds::App&)
{
}

void EngineClient::update()
{
  updateClient();

  if (mConnection.initialized()) {
    //if (mSendBuffer.size() > 0) {
    //  int size = mSendBuffer.size();
    //  mRawDataBuffer.resize(size);
    //  mSendBuffer.readRaw(mRawDataBuffer.data(), size);
    //  snappy::Compress(mRawDataBuffer.data(), size, &mCompressionBufferWrite);
    //  mConnection.sendMessage(mCompressionBufferWrite);
    //}

    if (mConnection.recvMessage(mCompressionBufferWrite)) {
      snappy::Uncompress(mCompressionBufferWrite.c_str(), mCompressionBufferWrite.size(), &mCompressionBufferRead);
      mReceiveBuffer.clear();
      mReceiveBuffer.addRaw(mCompressionBufferRead.c_str(), mCompressionBufferRead.size());
    }
  }

  // Receive and handle server data
  const char      size = static_cast<char>(mBlobRegistry.mReader.size());
  while (mReceiveBuffer.canRead<char>()) {
    const char  token = mReceiveBuffer.read<char>();
//    if (token != HEADER_BLOB) std::cout << "receive blob " << (int)(token) << std::endl;
    if (token > 0 && token < size) mBlobRegistry.mReader[token](mBlobReader);
  }
}

void EngineClient::draw()
{
  drawClient();
}

void EngineClient::receiveHeader(ds::DataBuffer& data)
{
//  std::cout << "EngineClient::receiveHeader()" << std::endl;
}

void EngineClient::receiveCommand(ds::DataBuffer& data)
{
}

} // namespace ds
