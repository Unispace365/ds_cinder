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
EngineClient::EngineClient(ds::App& app, const ds::cfg::Settings& settings, const std::vector<int>* roots)
    : inherited(app, settings, roots)
    , mLoadImageService(mLoadImageThread)
//    , mConnection(NumberOfNetworkThreads)
    , mSender(mSendConnection)
    , mReceiver(mReceiveConnection)
    , mBlobReader(mReceiver.getData(), *this)
    , mState(nullptr)
{
  // NOTE:  Must be EXACTLY the same items as in EngineServer, in same order,
  // so that the BLOB ids match.
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});

  try {
//   mConnection.initialize(false, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
   mSendConnection.initialize(true, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:listen_port")));
   mReceiveConnection.initialize(false, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
  } catch(std::exception &e) {
    DS_LOG_ERROR_M("EngineClient() initializing 0MQ: " << e.what(), ds::ENGINE_LOG);
  }

  setState(mBlankState);
}

EngineClient::~EngineClient()
{
  // It's important to clean up the sprites before the services go away
	clearAllSprites();
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

  // Every update, receive data
  if (!mReceiver.receiveAndHandle(mBlobRegistry, mBlobReader)) {
    // If I didn't receive any data, then don't send any data. This is
    // pretty important -- 0MQ will buffer sent commands if there's
    // no one to receive them. There isn't a way to ask the socket how
    // many connections there are, so we rely on the fact that the
    // server sounds out data each frame to tell us if there's someone
    // to receive anything.
    return;
  }

  // Oh this is interesting... There can be more data in the pipe
  // after handling, so make sure to slurp it all up, or else you
  // can end up in a situation where the render lags behind the world
  // by a couple seconds. For now, limit the amount of blocks I might
  // slurp up, to guarantee I don't go into an infinite loop on some
  // weird condition.
  if (mReceiveConnection.canRecv()) {
    mReceiver.receiveAndHandle(mBlobRegistry, mBlobReader);
    if (mReceiveConnection.canRecv()) mReceiver.receiveAndHandle(mBlobRegistry, mBlobReader);
  }

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
  if (data.canRead<int>()) {
    const int frame = data.read<int>();
//    std::cout << "receive frame=" << frame << std::endl;
  }
  // Terminator
  if (data.canRead<char>()) {
    data.read<char>();
  }
}

void EngineClient::receiveCommand(ds::DataBuffer& data)
{
  char            cmd;
  while (data.canRead<char>() && (cmd=data.read<char>()) != ds::TERMINATOR_CHAR) {
    if (cmd == CMD_SERVER_SEND_WORLD) {
      std::cout << "receive world" << std::endl;
			clearAllSprites();
      setState(mRunningState);
    }
  }
}

void EngineClient::setState(State& s)
{
  if (&s == mState) return;
  
  s.begin(*this);
  mState = &s;
}

/**
 * EngineClient::State
 */
EngineClient::State::State()
{
}

void EngineClient::State::begin(EngineClient&)
{
}

/**
 * EngineClient::RunningState
 */
EngineClient::RunningState::RunningState()
{
}

void EngineClient::RunningState::update(EngineClient& engine)
{
}

/**
 * EngineClient::BlankState
 */
EngineClient::BlankState::BlankState()
  : mSendFrame(0)
{
}

void EngineClient::BlankState::begin(EngineClient&)
{
  mSendFrame = 0;
}

void EngineClient::BlankState::update(EngineClient& engine)
{
  if (mSendFrame <= 0) {
    EngineSender::AutoSend  send(engine.mSender);
    ds::DataBuffer&   buf = send.mData;
    buf.add(COMMAND_BLOB);
    buf.add(CMD_CLIENT_REQUEST_WORLD);
    buf.add(ds::TERMINATOR_CHAR);

    mSendFrame = 10;
  }
  --mSendFrame;
}

} // namespace ds
