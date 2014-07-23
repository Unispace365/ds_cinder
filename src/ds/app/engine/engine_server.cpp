#include "ds/app/engine/engine_server.h"

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
 * \class ds::AbstractEngineServer
 */
AbstractEngineServer::AbstractEngineServer(	ds::App& app, const ds::cfg::Settings& settings,
											ds::EngineData& ed, const ds::RootList& roots)
    : inherited(app, settings, ed, roots)
//    , mConnection(NumberOfNetworkThreads)
    , mSender(mSendConnection)
    , mReceiver(mReceiveConnection)
    , mBlobReader(mReceiver.getData(), *this)
    , mState(nullptr)
{
	// NOTE:  Must be EXACTLY the same items as in EngineClient, in same order,
	// so that the BLOB ids match.
	HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveHeader(r.mDataBuffer);});
	COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {this->receiveCommand(r.mDataBuffer);});

	try {
		if (settings.getBool("server:connect", 0, true)) {
			mSendConnection.initialize(true, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
			mReceiveConnection.initialize(false, settings.getText("server:ip"), ds::value_to_string(settings.getInt("server:listen_port")));
		}
	} catch (std::exception &e) {
		DS_LOG_ERROR_M("EngineServer() initializing 0MQ: " << e.what(), ds::ENGINE_LOG);
	}

	setState(mSendWorldState);
}

AbstractEngineServer::~AbstractEngineServer() {
	// It's important to clean up the sprites before the services go away
	clearAllSprites();
}

void AbstractEngineServer::installSprite( const std::function<void(ds::BlobRegistry&)>& asServer,
										 const std::function<void(ds::BlobRegistry&)>& asClient) {
	if (asServer) asServer(mBlobRegistry);
}

void AbstractEngineServer::setup(ds::App& app) {
	inherited::setup(app);

	app.setupServer();
}

void AbstractEngineServer::setupTuio(ds::App& a) {
	tuio::Client &tuioClient = getTuioClient();
	tuioClient.registerTouches(&a);
	registerForTuioObjects(tuioClient);
	tuioClient.connect(mTuioPort);
}

void AbstractEngineServer::update() {
	mWorkManager.update();
	updateServer();

	mState->update(*this);
}

void AbstractEngineServer::draw() {
	drawServer();
}

void AbstractEngineServer::stopServices() {
	inherited::stopServices();
	mWorkManager.stopManager();
}

void AbstractEngineServer::receiveHeader(ds::DataBuffer& data) {
	char            id;
	while (data.canRead<char>() && (id=data.read<char>()) != ds::TERMINATOR_CHAR) {
		// Nothing in the header right now.
	}
}

void AbstractEngineServer::receiveCommand(ds::DataBuffer& data) {
	char            cmd;
	while (data.canRead<char>() && (cmd=data.read<char>()) != ds::TERMINATOR_CHAR) {
		if (cmd == CMD_CLIENT_REQUEST_WORLD) {
			setState(mSendWorldState);
		}
	}
}

void AbstractEngineServer::setState(State& s) {
	if (&s == mState) return;
  
	s.begin(*this);
	mState = &s;
}

/**
 * AbstractEngineServer::State
 */
AbstractEngineServer::State::State() {
}

void AbstractEngineServer::State::begin(AbstractEngineServer&) {
}

void AbstractEngineServer::State::addHeader(ds::DataBuffer& data, const int frame) {
    data.add(HEADER_BLOB);
    data.add(frame);
    data.add(ds::TERMINATOR_CHAR);
}

/**
 * EngineServer::RunningState
 */
EngineServer::RunningState::RunningState()
	: mFrame(0) {
}

void EngineServer::RunningState::begin(AbstractEngineServer&) {
	mFrame = 0;
}

void EngineServer::RunningState::update(AbstractEngineServer& engine) {
	// Send data to clients
	{
		EngineSender::AutoSend  send(engine.mSender);
		// Always send the header
		addHeader(send.mData, mFrame);
//		std::cout << "send frame=" << mFrame << std::endl;
		ui::Sprite                 &root = engine.getRootSprite();
		if (root.isDirty()) {
			root.writeTo(send.mData);
		}
	}

  // Receive data from clients
  engine.mReceiver.receiveAndHandle(engine.mBlobRegistry, engine.mBlobReader);

  mFrame++;
}

/**
 * EngineServer::SendWorldState
 */
EngineServer::SendWorldState::SendWorldState() {
}

void EngineServer::SendWorldState::update(AbstractEngineServer& engine) {
	{
		EngineSender::AutoSend  send(engine.mSender);
		DS_LOG_INFO_M("SEND WORLD " << std::time(0), ds::IO_LOG);
		// Always send the header
		addHeader(send.mData, -1);
		send.mData.add(COMMAND_BLOB);
		send.mData.add(CMD_SERVER_SEND_WORLD);
		send.mData.add(ds::TERMINATOR_CHAR);

		ui::Sprite                 &root = engine.getRootSprite();
		root.markTreeAsDirty();
		root.writeTo(send.mData);
	}

	engine.setState(engine.mRunningState);
}

/**
 * \class ds::EngineServer
 */
EngineServer::EngineServer(	ds::App& app, const ds::cfg::Settings& settings,
							ds::EngineData& ed, const ds::RootList& roots)
    : inherited(app, settings, ed, roots)
    , mLoadImageService(mLoadImageThread, mIpFunctions)
    , mRenderTextService(mRenderTextThread) {
}

EngineServer::~EngineServer() {
}

} // namespace ds
