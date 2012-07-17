#include "ds/app/engine_client.h"

#include "ds/ui/sprite/image.h"

namespace ds {
const char        HEADER_BLOCK = -1;
const char        COMMAND_BLOCK = -2;

/**
 * \class ds::EngineClient
 */
EngineClient::EngineClient(const ds::cfg::Settings& settings)
    : inherited(settings)
    , mLoadImageService(mLoadImageThread)
{
  mSpriteRegistry.add(HEADER_BLOCK, nullptr);
  mSpriteRegistry.add(COMMAND_BLOCK, nullptr);
  ds::ui::Sprite::addTo(mSpriteRegistry);
  ds::ui::Image::addTo(mSpriteRegistry);
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
  char          token;
  while (mReceiveBuffer.read(&token, 1)) {
    if (token == HEADER_BLOCK) {
      receiveHeader(mReceiveBuffer);
    } else if (token == COMMAND_BLOCK) {
      receiveCommand(mReceiveBuffer);
    }
  }
}

void EngineClient::draw()
{
  drawClient();
}

void EngineClient::receiveHeader(ds::DataBuffer& data)
{
}

void EngineClient::receiveCommand(ds::DataBuffer& data)
{
}

} // namespace ds
