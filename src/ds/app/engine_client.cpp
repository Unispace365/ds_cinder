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
    , mConnection(NumberOfNetworkThreads)
    , mSender(mConnection)
    , mReceiver(mConnection)
    , mBlobReader(mReceiver.getData(), *this)
    , mState(&mBlankState)
{
  // NOTE:  Must be EXACTLY the same items as in EngineServer, in same order,
  // so that the BLOB ids match.
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

  mReceiver.receiveAndHandle(mBlobRegistry, mBlobReader);
  mState->update(*this);
}

void EngineClient::draw()
{
  drawClient();
}

void EngineClient::stopServices()
{
  inherited::stopServices();
  mWorkManager.stopManager();
}

void EngineClient::receiveHeader(ds::DataBuffer& data)
{
//  std::cout << "EngineClient::receiveHeader()" << std::endl;
}

void EngineClient::receiveCommand(ds::DataBuffer& data)
{
  std::cout << "RECEIVE COMMAND" << std::endl;

  while (data.canRead<char>()) {
    const char    cmd = data.read<char>();
    if (cmd == CMD_SERVER_SEND_WORLD) {
      std::cout << "...WORLD!" << std::endl;
    }
  }

}

/**
 * EngineClient::State
 */
EngineClient::State::State()
{
}

/**
 * EngineClient::BlankState
 */
EngineClient::BlankState::BlankState()
{
}

void EngineClient::BlankState::update(EngineClient& engine)
{
  EngineSender::AutoSend  send(engine.mSender);
  std::cout << "Send world request" << std::endl;
  ds::DataBuffer&   buf = send.mData;
  buf.add(COMMAND_BLOB);
  buf.add(CMD_CLIENT_REQUEST_WORLD);
  buf.add(ds::TERMINATOR_CHAR);
}

} // namespace ds
