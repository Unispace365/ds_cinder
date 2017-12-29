#include "stdafx.h"

#include "ds/app/engine/engine_client.h"

#include "ds/app/engine/engine_io_defs.h"
#include "ds/app/engine/engine_data.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
#include "ds/ui/sprite/image.h"
#include "ds/util/string_util.h"
#include <cinder/Rand.h>
#include "snappy.h"

#include "ds/debug/computer_info.h"

namespace ds {

namespace {
char				HEADER_BLOB = 0;
char				COMMAND_BLOB = 0;
char				DELETE_SPRITE_BLOB = 0;

// Used for clients to get info to the server
char				CLIENT_STATUS_BLOB = 0;

// Used for clients to send mouse and/or touch input back to server
char				CLIENT_INPUT_BLOB = 0;
}

/**
 * \class ds::EngineClient
 */
char EngineClient::getClientStatusBlob() {
	return CLIENT_STATUS_BLOB;
}

EngineClient::EngineClient(	ds::App& app, ds::EngineSettings& settings,
							ds::EngineData& ed, const ds::RootList& roots)
		: inherited(app, settings, ed, roots)
		, mLoadImageService(*this, mIpFunctions)
		, mSender(mSendConnection, false)
		, mReceiver(mReceiveConnection, true)
		, mBlobReader(mReceiver.getData(), *this)
		, mSessionId(0)
		, mConnectionRenewed(false)
		, mServerFrame(-1)
		, mState(nullptr)
		, mIoInfo(*this)
{

	// NOTE:  Must be EXACTLY the same items as in EngineServer, in same order,
	// so that the BLOB ids match.
	HEADER_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveHeader(r.mDataBuffer);});
	COMMAND_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveCommand(r.mDataBuffer);});
	DELETE_SPRITE_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveDeleteSprite(r.mDataBuffer);});
	CLIENT_STATUS_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveClientStatus(r.mDataBuffer); });
	CLIENT_INPUT_BLOB = mBlobRegistry.add([this](BlobReader& r) {receiveClientInput(r.mDataBuffer); });
	mReceiver.setHeaderAndCommandIds(HEADER_BLOB, COMMAND_BLOB);
	
	try {
		if (settings.getBool("server:connect", 0, true)) {
			mSendConnection.initialize(true, settings.getString("server:ip"), ds::value_to_string(settings.getInt("server:listen_port")));
			mReceiveConnection.initialize(false, settings.getString("server:ip"), ds::value_to_string(settings.getInt("server:send_port")));
		}
	} catch(std::exception &e) {
		DS_LOG_ERROR_M("EngineClient::EngineClient() initializing UDP: " << e.what(), ds::ENGINE_LOG);
	}

	setState(mClientStartedState);
}

EngineClient::~EngineClient() {
	// It's important to clean up the sprites before the services go away
	clearRoots();
}

void EngineClient::installSprite( const std::function<void(ds::BlobRegistry&)>& asServer,
								  const std::function<void(ds::BlobRegistry&)>& asClient) {
	if (asClient) asClient(mBlobRegistry);
}

ds::sprite_id_t EngineClient::nextSpriteId() {
	// Clients never generate sprite IDs, they are always assigned from a blob.
	return 0;
}

void EngineClient::setup(ds::App& app) {
	inherited::setup(app);
}

void EngineClient::update() {
	mWorkManager.update();
	updateClient();
	mComputerInfo->update();

	if (!mConnectionRenewed && 
		(mReceiver.hasLostConnection() || !mSendConnection.initialized())
		){
		// This can happen because the network connection drops, so
		// refresh it, and let the world now I'm ready again.
		mReceiveConnection.renew();
		mSendConnection.renew();
		mReceiver.clearLostConnection();

		if(mReceiveConnection.initialized() && mSendConnection.initialized()){
			mConnectionRenewed = true;
		}

		setState(mClientStartedState);
		return;
	}


	// Every update, receive data
	mReceiver.setHeaderAndCommandOnly(mState->getHeaderAndCommandOnly());

	// Don't change state or take any action if there's no data waiting
	if(!mReceiver.receiveBlob()) return;

	// Run through all the blobs we just 
	while(true) {
		bool moreData = false;

		// there's an edge case where we're just listening for the fresh world
		// If that's the case, then don't handle the rest of the blobs this frame, handle those next frame (the receiver keeps track of stuff)
		if(!mReceiver.handleBlob(mBlobRegistry, mBlobReader, moreData)){
			return;
		}

		if(!moreData) break;
	}

	mConnectionRenewed = false;

	mState->update(*this);
}

void EngineClient::draw() {
	drawClient();
}

void EngineClient::stopServices() {
	inherited::stopServices();
	mWorkManager.stopManager();
}

int EngineClient::getBytesRecieved(){
	return mReceiveConnection.getReceivedBytes();
}

int EngineClient::getBytesSent(){
	return mSendConnection.getSentBytes();
}

void EngineClient::receiveHeader(ds::DataBuffer& data) {
	if (data.canRead<int32_t>()) {
		mServerFrame = data.read<int32_t>();
	} else {
		DS_LOG_WARNING_M("EngineClient::receiveHeader() invalid server frame. This is likely a net communication issue, packets lost, etc.", ds::IO_LOG);
	}
	// Terminator
	if (data.canRead<char>()) {
		data.read<char>();
	} else {
		DS_LOG_WARNING_M("EngineClient::receiveHeader() No terminator found for header!", ds::IO_LOG);
	}
}

void EngineClient::receiveCommand(ds::DataBuffer& data) {
	char            cmd;
	while (data.canRead<char>() && (cmd=data.read<char>()) != ds::TERMINATOR_CHAR) {
		if (cmd == CMD_SERVER_SEND_WORLD) {
			DS_LOG_INFO_M("Receive world, sessionid=" << mSessionId, ds::IO_LOG);
			clearAllSprites(false);

			if (mSessionId < 1) {
				setState(mClientStartedState);
			} else {
				// Make sure the rest of the blobs are handled
				mReceiver.setHeaderAndCommandOnly(false);
				setState(mRunningState);
			}
		} else if(cmd == CMD_CLIENT_STARTED_REPLY) {
			DS_LOG_INFO_M("Receive ClientStartedReply", ds::IO_LOG);
			onClientStartedReplyCommand(data);
		}
	}
}

void EngineClient::receiveDeleteSprite(ds::DataBuffer& data) {
	// First data is the count
	if (!data.canRead<size_t>()) return;

	size_t		size = data.read<size_t>();
	for (size_t k=0; k<size; ++k) {
		if (data.canRead<sprite_id_t>()) {
			const sprite_id_t	id = data.read<sprite_id_t>();

			if (!mSprites.empty()) {
				auto f = mSprites.find(id);
				if (f != mSprites.end()) {
					// Once I delete this item, mSprites will have been updated,
					// with f->second and all children removed, so do not reference f again.
					if (f->second) f->second->release();
				}
			}
		} else {
			break;
		}
	}
	if (!data.canRead<char>()) {
		DS_LOG_ERROR("EngineClient::receiveDeleteSprite() no space for an ending terminator");
		return;
	}
	const char		cmd(data.read<char>());
	if (cmd != ds::TERMINATOR_CHAR) {
		DS_LOG_ERROR("EngineClient::receiveDeleteSprite() no ending terminator");
	}
}

void EngineClient::receiveClientStatus(ds::DataBuffer& data) {
	// Whaaaat? Server should never be sending this.
	while (data.canRead<char>()) {
		const char		cmd(data.read<char>());
		if (cmd == ds::TERMINATOR_CHAR) return;
	}
}

void EngineClient::receiveClientInput(ds::DataBuffer& data) {
	// Whaaaat? Server should never be sending this.
	while(data.canRead<char>()) {
		const char		cmd(data.read<char>());
		if(cmd == ds::TERMINATOR_CHAR) return;
	}
}

void EngineClient::onClientStartedReplyCommand(ds::DataBuffer& data) {
	clearRoots();
	
	char					cmd;
	while (data.canRead<char>() && (cmd=data.read<char>()) != ds::TERMINATOR_CHAR) {
		if (cmd == ATT_CLIENT) {
			char			att;
			std::string		guid;
			int32_t			sessionid(0);
			unsigned int	chunkerId(0);
			while (data.canRead<char>() && (att=data.read<char>()) != ds::TERMINATOR_CHAR) {
				if (att == ATT_GLOBAL_ID) {
					guid = data.read<std::string>();
				} else if (att == ATT_SESSION_ID) {
					sessionid = data.read<int32_t>();
				} else if(att == ATT_ROOTS){
					std::vector<RootList::Root> roots;
					int numRoots = data.read<int32_t>();
					for(int i = 0; i < numRoots; i++){
						RootList::Root root = RootList::Root();
						int rootId = data.read<int32_t>();
						int typey = data.read<int32_t>();
						if(typey == RootList::Root::kOrtho){
							root.mType = RootList::Root::kOrtho;
						} else if(typey == RootList::Root::kPerspective){
							root.mType = RootList::Root::kPerspective;
						} else {
							DS_LOG_ERROR("Got an invalid root type! " << typey);
							continue;
						}


						root.mRootId = rootId;
						roots.push_back(root);
					}

					createClientRoots(roots);
				}
			}
			if (guid == mIoInfo.mGlobalId) {
				mSessionId = sessionid;
				mSender.setPacketNumber(chunkerId);
				setState(mBlankState);
			}
		} 
	}
}

void EngineClient::setState(State& s) {
	if (&s == mState) return;
  
	s.begin(*this);
	mState = &s;
}

void EngineClient::handleMouseTouchBegin(const ci::app::MouseEvent& e, int id){
	sendMouseTouch(0, e.getPos());
}

void EngineClient::handleMouseTouchMoved(const ci::app::MouseEvent& e, int id){
	sendMouseTouch(1, e.getPos());
}

void EngineClient::handleMouseTouchEnded(const ci::app::MouseEvent& e, int id){
	sendMouseTouch(2, e.getPos());
}

void EngineClient::sendMouseTouch(const int phase, const ci::ivec2 pos){	
	ci::vec2 worldPoint = pos;
	
	EngineSender::AutoSend  send(mSender);
	ds::DataBuffer&   buf = send.mData;
	buf.add(CLIENT_INPUT_BLOB);
	buf.add(phase); 
	buf.add(-1); //id
	buf.add(worldPoint.x);
	buf.add(worldPoint.y);
	buf.add(ds::TERMINATOR_CHAR);
}

/**
 * EngineClient::State
 */
EngineClient::State::State() {
}

void EngineClient::State::begin(EngineClient&) {
}

/**
 * EngineClient::RunningState
 */
EngineClient::RunningState::RunningState() {
}

void EngineClient::RunningState::begin(EngineClient &c) {
	DS_LOG_INFO_M("RunningState", ds::IO_LOG);
	c.mServerFrame = -1;
}

void EngineClient::RunningState::update(EngineClient &e) {
	EngineSender::AutoSend  send(e.mSender);
	ds::DataBuffer&   buf = send.mData;
	buf.add(COMMAND_BLOB);
	buf.add(CMD_CLIENT_RUNNING);
	buf.add(ATT_SESSION_ID);
	buf.add(e.mSessionId);
	buf.add(ATT_FRAME);
	buf.add(e.mServerFrame);
	buf.add(ds::TERMINATOR_CHAR);

	const size_t count(e.getRootCount());
	for (int k=0; k<count; ++k) {
		if(!e.getRootBuilder(k).mSyncronize) continue;
		ui::Sprite&	s(e.getRootSprite(k));
		s.writeClientTo(buf);
	}

	//DS_LOG_INFO_M("RunningState send reply frame=" << e.mServerFrame, ds::IO_LOG);
}

/**
 * EngineClient::ClientStartedState
 */
EngineClient::ClientStartedState::ClientStartedState()
		: mSendFrame(0) {
}

void EngineClient::ClientStartedState::begin(EngineClient&) {
	DS_LOG_INFO_M("ClientStartedState", ds::IO_LOG);
	mSendFrame = 0;
}

void EngineClient::ClientStartedState::update(EngineClient& engine) {
	if(!engine.mSendConnection.initialized()){
		engine.mSendConnection.renew();
	}
	if (mSendFrame <= 0) {
		EngineSender::AutoSend  send(engine.mSender);
		ds::DataBuffer&   buf = send.mData;
		buf.add(COMMAND_BLOB);
		buf.add(CMD_CLIENT_STARTED);
		buf.add(ATT_GLOBAL_ID);
		buf.add(engine.mIoInfo.mGlobalId);
		buf.add(ds::TERMINATOR_CHAR);

		// Randomize the amount of time to wait for a retry. 
		// If there are multiple clients that started at the same time, we could be flooding the server with new world requests
		mSendFrame = ci::randInt(60, 300);

		DS_LOG_INFO_M("Send CMD_CLIENT_STARTED", ds::IO_LOG);
	}
	--mSendFrame;
}

/**
 * EngineClient::BlankState
 */
EngineClient::BlankState::BlankState()
		: mSendFrame(0) {
}

void EngineClient::BlankState::begin(EngineClient&) {
	DS_LOG_INFO_M("BlankState", ds::IO_LOG);
	mSendFrame = 0;
}

void EngineClient::BlankState::update(EngineClient& engine) {
	if (mSendFrame <= 0) {
		EngineSender::AutoSend  send(engine.mSender);
		ds::DataBuffer&   buf = send.mData;
		buf.add(COMMAND_BLOB);
		buf.add(CMD_CLIENT_REQUEST_WORLD);
		buf.add(ds::TERMINATOR_CHAR);

		// Randomize the amount of time to wait for a retry. 
		// If there are multiple clients that started at the same time, we could be flooding the server with new world requests
		mSendFrame = ci::randInt(30, 240);
	}
	--mSendFrame;
}

} // namespace ds
