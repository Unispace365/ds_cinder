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
EngineServer::EngineServer(ds::App& app, const ds::cfg::Settings& settings)
    : inherited(app, settings)
    , mLoadImageService(mLoadImageThread)
    , mConnection(NumberOfNetworkThreads)
    , mSender(mConnection)
    , mReceiver(mConnection)
    , mBlobReader(mReceiver.getData(), *this)
    , mState(&mRunningState)
{
  // NOTE:  Must be EXACTLY the same items as in EngineClient, in same order,
  // so that the BLOB ids match.
  HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
  COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});

  try {
    mConnection.initialize(true, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
  } catch (std::exception &e) {
    DS_LOG_ERROR_M("EngineServer() initializing 0MQ: " << e.what(), ds::ENGINE_LOG);
  }
}

EngineServer::~EngineServer()
{
  // It's important to clean up the sprites before the services go away
  mRootSprite.clearChildren();
}

void EngineServer::installSprite( const std::function<void(ds::BlobRegistry&)>& asServer,
                                  const std::function<void(ds::BlobRegistry&)>& asClient)
{
  if (asServer) asServer(mBlobRegistry);
}

void EngineServer::setup(ds::App& app)
{
  inherited::setup(app);

  app.setupServer();
}

void EngineServer::setupTuio(ds::App& a)
{
  tuio::Client &tuioClient = getTuioClient();
  tuioClient.registerTouches(&a);
  tuioClient.connect(mTuioPort);
}

void EngineServer::update()
{
  mWorkManager.update();
  updateServer();

  mState->update(*this);
}

void EngineServer::draw()
{
  drawServer();
}

void EngineServer::stopServices()
{
  inherited::stopServices();
  mWorkManager.stopManager();
}

void EngineServer::receiveHeader(ds::DataBuffer& data)
{
}

void EngineServer::receiveCommand(ds::DataBuffer& data)
{
  std::cout << "RECEIVE at" << time(0) << std::endl;
  while (data.canRead<char>()) {
    const char    cmd = data.read<char>();
    if (cmd == CMD_CLIENT_REQUEST_WORLD) {
      std::cout << "Client REQUEST FOR WORLD at " << time(0) << std::endl;
      mState = &mSendWorldState;
    }
  }
}

/**
 * EngineServer::State
 */
EngineServer::State::State()
{
}

/**
 * EngineServer::RunningState
 */
EngineServer::RunningState::RunningState()
{
}

void EngineServer::RunningState::update(EngineServer& engine)
{
  // Send data to clients
  {
    EngineSender::AutoSend  send(engine.mSender);
    // Always send the header
    send.mData.add(HEADER_BLOB);
    send.mData.add(ds::TERMINATOR_CHAR);

    ui::Sprite                 &root = engine.getRootSprite();
    if (root.isDirty()) {
      root.writeTo(send.mData);
    }
  }

  // Receive data from clients
  engine.mReceiver.receiveAndHandle(engine.mBlobRegistry, engine.mBlobReader);
}

/**
 * EngineServer::SendWorldState
 */
EngineServer::SendWorldState::SendWorldState()
{
}

void EngineServer::SendWorldState::update(EngineServer& engine)
{
  // Send data to clients
  {
    EngineSender::AutoSend  send(engine.mSender);
    std::cout << "SEND WORLD " << std::time(0) << std::endl;
    // Always send the header
    send.mData.add(HEADER_BLOB);
    send.mData.add(ds::TERMINATOR_CHAR);
    send.mData.add(COMMAND_BLOB);
    send.mData.add(CMD_SERVER_SEND_WORLD);
    send.mData.add(ds::TERMINATOR_CHAR);

    ui::Sprite                 &root = engine.getRootSprite();
    root.markTreeAsDirty();
    root.writeTo(send.mData);
  }

  engine.mState = &engine.mRunningState;
}

} // namespace ds
