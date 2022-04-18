#include "stdafx.h"

#include "ds/app/engine/engine_server.h"

#include <ds/app/engine/engine_io_defs.h>
#include "ds/app/app.h"
#include "ds/app/blob_reader.h"
#include "ds/debug/logger.h"
#include "ds/util/string_util.h"
#include "ds/debug/computer_info.h"
#include "ds/app/engine/engine_events.h"
#include "ds/content/content_wrangler.h"

namespace ds {

namespace {
char				HEADER_BLOB = 0;
char				COMMAND_BLOB = 0;
char				DELETE_SPRITE_BLOB = 0;

// Used for clients to get info to the server
char				CLIENT_STATUS_BLOB = 0;

// Used for clients to send mouse and/or touch input back to server
char				CLIENT_INPUT_BLOB = 0;

const char			TERMINATOR = 0;
}

using namespace ci;
using namespace ci::app;

/**
 * \class AbstractEngineServer
 */
AbstractEngineServer::AbstractEngineServer(	ds::App& app, ds::EngineSettings& settings,
											ds::EngineData& ed, const ds::RootList& roots, const int appMode)
	: ds::Engine(app, settings, ed, roots, appMode)
//    , mConnection(NumberOfNetworkThreads)
	, mSender(mSendConnection, true)
	, mReceiver(mReceiveConnection, false)
	, mBlobReader(mReceiver.getData(), *this)
	, mState(nullptr)
	, mContentWrangler(nullptr)
{
	// NOTE:  Must be EXACTLY the same items as in EngineClient, in same order,
	// so that the BLOB ids match.
	HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveHeader(r.mDataBuffer);});
	COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveCommand(r.mDataBuffer);});
	DELETE_SPRITE_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveDeleteSprite(r.mDataBuffer);});
	CLIENT_STATUS_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveClientStatus(r.mDataBuffer); });
	CLIENT_INPUT_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveClientInput(r.mDataBuffer); });

	try {
		if (settings.getBool("server:connect", 0, true)) {
			mSendConnection.initialize(true, settings.getString("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
			mReceiveConnection.initialize(false, settings.getString("server:ip"), ds::value_to_string(settings.getInt("server:listen_port")));
		}
	} catch (std::exception &e) {
		DS_LOG_ERROR_M("EngineServer() initializing connection: " << e.what(), ds::ENGINE_LOG);
	}

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
	Engine::setup(app);

	if(!mContentWrangler) {
		mContentWrangler = new ContentWrangler(*this);
	}
	if(mContentWrangler) {
		mContentWrangler->initialize();
	}

	setState(mSendWorldState);

	app.preServerSetup();
	app.setupServer();
}

void AbstractEngineServer::update() {
	mComputerInfo->update();
	mWorkManager.update();
	updateServer();

	mState->update(*this);
}

void AbstractEngineServer::draw() {
	drawServer();
}

void AbstractEngineServer::stopServices() {
	Engine::stopServices();
	mWorkManager.stopManager();
}

void AbstractEngineServer::spriteDeleted(const ds::sprite_id_t &id) {
	mState->spriteDeleted(id);
}

int AbstractEngineServer::getBytesRecieved(){
	return mReceiveConnection.getReceivedBytes();
}

int AbstractEngineServer::getBytesSent(){
	return mSendConnection.getSentBytes();
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
			DS_LOG_ERROR_M("Multiple servers: This app is running in server mode, but has received a command sent from another server. Check that there are not multiple servers sending to the same IP address", ds::IO_LOG);
		}
	}
}

void AbstractEngineServer::receiveDeleteSprite(ds::DataBuffer&) {
	// Whaaaat? Client should never be sending this.
}

void AbstractEngineServer::receiveClientStatus(ds::DataBuffer& data) {
	if (!data.canRead<sprite_id_t>()) {
		// Error, run to the next terminator
		while (data.canRead<char>()) {
			const char		cmd(data.read<char>());
			if (cmd == ds::TERMINATOR_CHAR) return;
		}	
	}

	// Find the sprite, and let it read
	const sprite_id_t		id(data.read<sprite_id_t>());
	ds::ui::Sprite*			s(findSprite(id));
	if(!s) {

		DS_LOG_WARNING_M("receiveClientStatus missing sprite id=" << id, ds::IO_LOG);

		auto it = std::find(mRunningState.mDeletedSprites.begin(), mRunningState.mDeletedSprites.end(), id);
		if(it != mRunningState.mDeletedSprites.end()){
			DS_LOG_INFO_M("Actually, it's cool, that sprite was just deleted. id=" << id, ds::IO_LOG);
		}


		// This can happen if the sprite has been deleted by the server but not yet by the client
		while(data.canRead<char>()) {
			const char		cmd(data.read<char>());
			if(cmd == ds::TERMINATOR_CHAR) return;
		}

	} else {
		s->readClientFrom(data);
	}

	
	// Verify we're at the end
	if(data.canRead<char>()) {
		const char			cmd(data.read<char>());
		if(cmd != ds::TERMINATOR_CHAR) {
			DS_LOG_WARNING_M("receiveClientStatus missing terminator. Got " << cmd << " instead", ds::IO_LOG);
		}
	}
}

void AbstractEngineServer::receiveClientInput(ds::DataBuffer& data) {
	if(!data.canRead<sprite_id_t>()) {
		// Error, run to the next terminator
		while(data.canRead<char>()) {
			const char		cmd(data.read<char>());
			if(cmd == ds::TERMINATOR_CHAR) return;
		}
	}

	const int				state(data.read<int>());
	const int				id(data.read<int>());
	const float				xp(data.read<float>());
	const float				yp(data.read<float>());

	std::vector<ci::app::TouchEvent::Touch> touches;
	touches.push_back(ci::app::TouchEvent::Touch(ci::vec2(xp, yp), ci::vec2(xp, yp), id, 0.0, nullptr));
	ds::ui::TouchEvent te = ds::ui::TouchEvent(getWindow(), touches, true);
	if(state == 0){
		injectTouchesBegin(te);
	} else if(state == 1){
		injectTouchesMoved(te);
	} else if(state == 2){
		injectTouchesEnded(te);
	}

	// Verify we're at the end
	if(data.canRead<char>()) {
		const char			cmd(data.read<char>());
		if(cmd != ds::TERMINATOR_CHAR) {
			DS_LOG_WARNING_M("receiveClientInput missing terminator", ds::IO_LOG);
		}
	}
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
	} else {
		DS_LOG_INFO_M("onClientStartedCommand didn't receive a valid guid or sessionid!", ds::IO_LOG);
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

void AbstractEngineServer::handleMouseTouchBegin(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchBegin(e, id);
}

void AbstractEngineServer::handleMouseTouchMoved(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchMoved(e, id);
}

void AbstractEngineServer::handleMouseTouchEnded(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchEnded(e, id);
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

void EngineServer::RunningState::begin(AbstractEngineServer& engine) {
	DS_LOG_INFO_M("RunningState", ds::IO_LOG);
	engine.getNotifier().notify(ds::app::EngineStateEvent(ds::app::EngineStateEvent::ENGINE_STATE_CLIENT_RUNNING));
	mFrame = 0;
	mDeletedSprites.clear();
}

void EngineServer::RunningState::update(AbstractEngineServer& engine) {
	if (engine.mReceiver.hasLostConnection()) {
		engine.mReceiveConnection.renew();
		engine.mSendConnection.renew();
		
		engine.mReceiver.clearLostConnection();
	}

	// Send data to clients
	{
		EngineSender::AutoSend  send(engine.mSender);
		// Always send the header
		addHeader(send.mData, mFrame);

		const size_t numRoots = engine.getRootCount();
		for(int i = 0; i < numRoots; i++){
			if(!engine.getRootBuilder(i).mSyncronize) continue;
			ds::ui::Sprite& rooty = engine.getRootSprite(i);
			if(rooty.isDirty()){
				rooty.writeTo(send.mData);
			}
		}

		if (!mDeletedSprites.empty()) {
			addDeletedSprites(send.mData);
			mDeletedSprites.clear();
		}
	}

	// this receive call pulls everything it can off the wire and caches it
	// if there was an error decoding the chunks, then go back to sending a full world
	if(!engine.mReceiver.receiveBlob(false)){
		engine.mReceiver.clearLostConnection();
		engine.setState(engine.mSendWorldState);
		return;
	}

	// now we can look through all the data we got and handle it
	while(true) {
		bool moreData = false;
		engine.mReceiver.handleBlob(engine.mBlobRegistry, engine.mBlobReader, moreData);
		if(!moreData) {
			break;
		}
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

void EngineServer::ClientStartedReplyState::begin(AbstractEngineServer& engine) {
	DS_LOG_INFO_M("ClientStartedReplyState", ds::IO_LOG);
	engine.getNotifier().notify(ds::app::EngineStateEvent(ds::app::EngineStateEvent::ENGINE_STATE_CLIENT_STARTED));
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

				send.mData.add(ATT_ROOTS);
				size_t rootCount = engine.getRootCount();
				int numActualRoots = 0;
				std::vector<RootList::Root> roots;

				for(size_t i = 0; i < rootCount; i++){
					if(!engine.getRootBuilder(i).mSyncronize) continue;
					numActualRoots++;
					RootList::Root newRoot = RootList::Root();
					newRoot.mRootId = engine.getRootBuilder(i).mRootId;
					newRoot.mType = engine.getRootBuilder(i).mType;
					roots.push_back(newRoot);
				}
				if(numActualRoots > 0){
					send.mData.add(numActualRoots);
					for(int i = 0; i < numActualRoots; i++){
						send.mData.add(roots[i].mRootId);
						send.mData.add(roots[i].mType);
					}
				} else {
					send.mData.add(0); // no roots? well, whatever
				}

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

void EngineServer::SendWorldState::begin(AbstractEngineServer& engine) {
	DS_LOG_INFO_M("SendWorldState", ds::IO_LOG);
	engine.getNotifier().notify(ds::app::EngineStateEvent(ds::app::EngineStateEvent::ENGINE_STATE_SEND_WORLD));
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

		const size_t numRoots = engine.getRootCount();
		for(size_t i = 0; i < numRoots; i++){
			if(!engine.getRootBuilder(i).mSyncronize) continue;
			ds::ui::Sprite& rooty = engine.getRootSprite(i);
			rooty.markTreeAsDirty();
			rooty.writeTo(send.mData);
			
		}
	}

	engine.setState(engine.mRunningState);
}

/**
 * \class EngineServer
 */
EngineServer::EngineServer(	ds::App& app, ds::EngineSettings& settings,
							ds::EngineData& ed, const ds::RootList& roots)
	: AbstractEngineServer(app, settings, ed, roots, SERVER_MODE)
{
}

EngineServer::~EngineServer() {
}

} // namespace ds
