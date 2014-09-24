#include "ds/app/engine/engine_server.h"

#include <ds/app/engine/engine_io_defs.h>
#include "ds/app/app.h"
#include "ds/app/blob_reader.h"
#include <ds/app/error.h>
#include "ds/debug/logger.h"
#include "ds/util/string_util.h"

namespace ds {

namespace {
char				HEADER_BLOB = 0;
char				COMMAND_BLOB = 0;
char				DELETE_SPRITE_BLOB = 0;

const char			TERMINATOR = 0;
}

using namespace ci;
using namespace ci::app;

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
	mClients.setErrorChannel(&getChannel(ERROR_CHANNEL));
	// NOTE:  Must be EXACTLY the same items as in EngineClient, in same order,
	// so that the BLOB ids match.
	HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveHeader(r.mDataBuffer);});
	COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveCommand(r.mDataBuffer);});
	DELETE_SPRITE_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveDeleteSprite(r.mDataBuffer);});

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
	if (ds::ui::TouchMode::hasTuio(mTouchMode)) {
		ci::tuio::Client &tuioClient = getTuioClient();
		tuioClient.registerTouches(&a);
		registerForTuioObjects(tuioClient);
		tuioClient.connect(mTuioPort);
	}
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

void AbstractEngineServer::spriteDeleted(const ds::sprite_id_t &id) {
	mState->spriteDeleted(id);
}

void AbstractEngineServer::receiveHeader(ds::DataBuffer& data) {
	char            id;
	while (data.canRead<char>() && (id=data.read<char>()) != ds::TERMINATOR_CHAR) {
		// Nothing in the header right now.
	}
}

void AbstractEngineServer::receiveCommand(ds::DataBuffer &data) {
	char            cmd;
	while (data.canRead<char>() && (cmd=data.read<char>()) != ds::TERMINATOR_CHAR) {
		if (cmd == CMD_CLIENT_STARTED) {
			DS_LOG_INFO_M("CMD_CLIENT_STARTED", ds::IO_LOG);
			onClientStartedCommand(data);
		} else if (cmd == CMD_CLIENT_RUNNING) {
			onClientRunningCommand(data);
		} else if (cmd == CMD_CLIENT_REQUEST_WORLD) {
			DS_LOG_INFO_M("CMD_CLIENT_REQUEST_WORLD", ds::IO_LOG);
			setState(mSendWorldState);
		} else if (cmd == CMD_SERVER_SEND_WORLD) {
			DS_LOG_INFO_M("CMD_SERVER_SEND_WORLD", ds::IO_LOG);
			static const ErrorRef		MULTIPLE_SERVERS_ERROR(ErrorRef::getNextId(), L"Multiple servers", L"This app is running in server mode, but has received a command sent from another server. Check that there are not multiple servers sending to the same IP address");
			getChannel(ERROR_CHANNEL).notify(AddErrorEvent(MULTIPLE_SERVERS_ERROR));
		}
	}
}

void AbstractEngineServer::receiveDeleteSprite(ds::DataBuffer&) {
	// Whaaaat? Client should never be sending this.
}

void AbstractEngineServer::onClientStartedCommand(ds::DataBuffer &data) {
	if (!data.canRead<char>()) return;
	char				att = data.read<char>();
	if (att != ATT_GLOBAL_ID) return;
	const std::string	guid = data.read<std::string>();
	const int32_t		sessionid = mClients.startClient(guid);
	if (sessionid > 0) {
		DS_LOG_INFO_M("onClientStartedCommand guid=" << guid, ds::IO_LOG);

		mClientStartedReplyState.mClients.push_back(sessionid);
		setState(mClientStartedReplyState);
	}
}

void AbstractEngineServer::onClientRunningCommand(ds::DataBuffer &data) {
	if (!data.canRead<char>()) return;

	// Session ID
	char				att = data.read<char>();
	if (att != ATT_SESSION_ID) return;
	const int32_t		session_id = data.read<int32_t>();

	// Frame
	att = data.read<char>();
	if (att != ATT_FRAME) return;
	const int32_t		frame = data.read<int32_t>();

	mClients.reportingIn(session_id, frame);
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
	mDeletedSprites.reserve(128);
}

void EngineServer::RunningState::begin(AbstractEngineServer&) {
	DS_LOG_INFO_M("RunningState", ds::IO_LOG);
	mFrame = 0;
	mDeletedSprites.clear();
}

void EngineServer::RunningState::update(AbstractEngineServer& engine) {
	if (engine.mReceiver.hasLostConnection()) {
//DS_LOG_INFO_M("server receiver lost connection", ds::IO_LOG);
		engine.mReceiveConnection.renew();
		engine.mReceiver.clearLostConnection();
	}

	// Send data to clients
	{
		EngineSender::AutoSend  send(engine.mSender);
		// Always send the header
		addHeader(send.mData, mFrame);
//		DS_LOG_INFO_M("running frame=" << mFrame, ds::IO_LOG);
		ui::Sprite                 &root = engine.getRootSprite();
		if (root.isDirty()) {
			root.writeTo(send.mData);
		}
		if (!mDeletedSprites.empty()) {
			addDeletedSprites(send.mData);
			mDeletedSprites.clear();
		}
	}

	// Handle data from all the clients. This high number is used
	// to compensate for catching up when things cause me to get
	// behind (which could be as simple as LogMeIn taking over a
	// machine). It might be that we just want to wait until all
	// registered clients have reported the current frame.
	int32_t		limit = 100;
	while (engine.mReceiveConnection.canRecv()) {
		engine.mReceiver.receiveAndHandle(engine.mBlobRegistry, engine.mBlobReader);
		if (--limit <= 0) break;
	}

	// Track how far behind any clients are
	engine.mClients.compare(mFrame);

	mFrame++;
}

void EngineServer::RunningState::spriteDeleted(const ds::sprite_id_t &id) {
	try {
		mDeletedSprites.push_back(id);
	} catch (std::exception const&) {
	}
}

void EngineServer::RunningState::addDeletedSprites(ds::DataBuffer &data) const {
	if (mDeletedSprites.empty()) return;

    data.add(DELETE_SPRITE_BLOB);
	data.add(mDeletedSprites.size());
	for (auto it=mDeletedSprites.begin(), end=mDeletedSprites.end(); it!=end; ++it) {
		data.add(*it);
	}
    data.add(ds::TERMINATOR_CHAR);
}

/**
 * EngineServer::ClientStartedReplyState
 */
EngineServer::ClientStartedReplyState::ClientStartedReplyState() {
	mClients.reserve(8);
}

void EngineServer::ClientStartedReplyState::clear() {
	mClients.clear();
}

void EngineServer::ClientStartedReplyState::begin(AbstractEngineServer&) {
	DS_LOG_INFO_M("ClientStartedReplyState", ds::IO_LOG);
}

void EngineServer::ClientStartedReplyState::update(AbstractEngineServer& engine) {
	{
		EngineSender::AutoSend  send(engine.mSender);
		DS_LOG_INFO_M("Send ClientStartedReply " << std::time(0), ds::IO_LOG);
		// Always send the header
		addHeader(send.mData, -1);
		send.mData.add(COMMAND_BLOB);
		send.mData.add(CMD_CLIENT_STARTED_REPLY);
		// Send each client
		for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
			const EngineClientList::State*	s(engine.mClients.findClient(*it));
			if (s) {
				send.mData.add(ATT_CLIENT);
				send.mData.add(ATT_GLOBAL_ID);
				send.mData.add(s->mGuid);
				send.mData.add(ATT_SESSION_ID);
				send.mData.add(s->mSessionId);
				send.mData.add(ds::TERMINATOR_CHAR);
			}
		}

		send.mData.add(ds::TERMINATOR_CHAR);
	}

	clear();
	engine.setState(engine.mSendWorldState);
}

/**
 * EngineServer::SendWorldState
 */
EngineServer::SendWorldState::SendWorldState() {
}

void EngineServer::SendWorldState::begin(AbstractEngineServer&) {
	DS_LOG_INFO_M("SendWorldState", ds::IO_LOG);
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
