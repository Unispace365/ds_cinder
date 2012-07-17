#include "ds/app/engine_client.h"

#include "ds/ui/sprite/image.h"
#include "ds/util/string_util.h"
#include "snappy.h"

namespace ds {

namespace {
char              VERIFIER_BLOB = 0;
char              HEADER_BLOB = 0;
char              COMMAND_BLOB = 0;
}

/**
 * \class ds::EngineClient
 */
EngineClient::EngineClient(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
    , mBlobReader(mReceiveBuffer, *this)
    , mConnection(NumberOfNetworkThreads)
{
  VERIFIER_BLOB = mBlobRegistry.add([this](BlobReader& r) {std::cout << "verify" << std::endl;});
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});

  mConnection.initialize(false, settings.getText("server:ip", 0, "239.255.20.20"), ds::value_to_string(settings.getInt("server:send_port", 0, 8000)));
}

ds::sprite_id_t EngineClient::nextSpriteId()
{
  // Clients never generate sprite IDs, they are always assigned from a blob.
  return 0;
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

  if (mConnection.initialized()) {
    //if (mSendBuffer.size() > 0) {
    //  int size = mSendBuffer.size();
    //  mRawDataBuffer.resize(size);
    //  mSendBuffer.readRaw(mRawDataBuffer.ptr(), size);
    //  snappy::Compress(mRawDataBuffer.constPtr(), size, &mCompressionBufferWrite);
    //  mConnection.sendMessage(mCompressionBufferWrite);
    //}

    if (mConnection.recvMessage(mCompressionBufferWrite)) {
      snappy::Uncompress(mCompressionBufferWrite.c_str(), mCompressionBufferWrite.size(), &mCompressionBufferRead);
      mReceiveBuffer.clear();
      mReceiveBuffer.addRaw(mCompressionBufferRead.c_str(), mCompressionBufferRead.size());
    }
  }

  // Receive and handle server data
  const char    size = static_cast<char>(mBlobRegistry.mReader.size());
  char          token;
  while (mReceiveBuffer.read(&token, 1)) {
    if (token > 0 && token < size) mBlobRegistry.mReader[token](mBlobReader);
  }
}

void EngineClient::draw()
{
  drawClient();
}

void EngineClient::receiveHeader(ds::DataBuffer& data)
{
  std::cout << "EngineClient::receiveHeader()" << std::endl;
}

void EngineClient::receiveCommand(ds::DataBuffer& data)
{
}

} // namespace ds
