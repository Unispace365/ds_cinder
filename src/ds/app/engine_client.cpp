#include "ds/app/engine_client.h"

namespace ds {

namespace {
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
{
  // NOTE:  Must be EXACTLY the same items as in EngineServer
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});
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
