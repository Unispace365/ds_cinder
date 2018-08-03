#include "stdafx.h"

#include "ds/app/engine/engine_io.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/debug/logger.h"
#include "ds/util/string_util.h"
#include "snappy.h"
#include <cinder/Rand.h>

#include "ds/network/packet_chunker.h"

namespace ds {

/**
 * \class EngineSender
 */
EngineSender::EngineSender(ds::NetConnection& con, const bool useChunker)
		: mConnection(con) 
		, mPacketId(0)
		, mUseChunker(useChunker)
{
}

void EngineSender::setPacketNumber(unsigned int packetId){
	mPacketId = packetId;
}

/**
 * \class AutoSend
 */
EngineSender::AutoSend::AutoSend(EngineSender& sender)
		: mData(sender.mSendBuffer)
		, mSender(sender) {
  mData.clear();
}

EngineSender::AutoSend::~AutoSend() {
	// Send data to client
	if (!mSender.mConnection.initialized()) return;
	if (mData.size() < 1) return;

	const size_t size = mData.size();
	mSender.mRawDataBuffer.setSize(size);
	mData.readRaw(mSender.mRawDataBuffer.data(), size);
	snappy::Compress(mSender.mRawDataBuffer.data(), size, &mSender.mCompressionBuffer);

	std::vector<std::string> chunks;

	if(mSender.mUseChunker){

		mSender.mPacketId++;
		ds::net::Chunker chunker;
		chunker.Chunkify(mSender.mCompressionBuffer, mSender.mPacketId, chunks);
	} else {
		chunks.push_back(mSender.mCompressionBuffer);
	}

	for (auto it : chunks){
		mSender.mConnection.sendMessage(it);
	}

	mData.clear();
}

/**
 * \class EngineReceiver
 */
EngineReceiver::EngineReceiver(ds::NetConnection& con, const bool useChunker)
		: mConnection(con)
		, mHeaderId(0)
		, mCommandId(0)
		, mHeaderAndCommandOnly(false)
		, mUseChunker(useChunker)
		, mNoDataCount(0) {
	setHeaderAndCommandOnly();
}

void EngineReceiver::setHeaderAndCommandIds(const char header, const char command) {
	mHeaderId = header;
	mCommandId = command;
}

void EngineReceiver::setHeaderAndCommandOnly(const bool b) {
	mHeaderAndCommandOnly = b;
}

ds::DataBuffer& EngineReceiver::getData() {
	return mCurrentDataBuffer;
}

bool EngineReceiver::receiveBlob() {
	std::string recvBuffer;

	if(mUseChunker){
		while(mConnection.recvMessage(recvBuffer)) {
			mDechunker.addChunk(recvBuffer);
		}

		while(mDechunker.getAvailable() > 0) {
			std::string outBuf;
			bool validy = mDechunker.getNextGroup(outBuf);

			if(!validy) {
				DS_LOG_WARNING_M("EngineReceiver: Invalid chunk received. Expect a new world frame shortly.", ds::IO_LOG);
				return false;
			}

			snappy::Uncompress(outBuf.c_str(), outBuf.size(), &mCompressionBufferRead);
			mReceiveBuffers.push_back(mCompressionBufferRead);
		}
	} else {
		while(mConnection.recvMessage(recvBuffer)) {
			snappy::Uncompress(recvBuffer.c_str(), recvBuffer.size(), &mCompressionBufferRead);
			mReceiveBuffers.push_back(mCompressionBufferRead);
		}
	}

	if(mReceiveBuffers.empty()) {
		++mNoDataCount;
		//return false;
	}

	return true;
}

bool EngineReceiver::handleBlob(ds::BlobRegistry& registry, ds::BlobReader& reader, bool& morePacketsAvailable) {
	if(mReceiveBuffers.empty()) {
		++mNoDataCount;
		morePacketsAvailable = false;
		return false;
	}

	mNoDataCount = 0;

	mCurrentDataBuffer.clear(); 
	mCurrentDataBuffer.addRaw(mReceiveBuffers.front().c_str(), mReceiveBuffers.front().size());
	mReceiveBuffers.erase(mReceiveBuffers.begin());

	morePacketsAvailable = !mReceiveBuffers.empty();

	const size_t				receiveSize = mCurrentDataBuffer.size();
	const char					size = static_cast<char>(registry.mReader.size());
	while (mCurrentDataBuffer.canRead<char>()) {
		const char				token = mCurrentDataBuffer.read<char>();
		if (token > 0 && token < size) {
			// If we're doing header and command only, as soon as we hit a
			// non-header, non-command, we need to bail
			if (mHeaderAndCommandOnly) {
				if (token == mHeaderId || token == mCommandId) {
					registry.mReader[token](reader);
				} else {
					return true;
				}
			} else {
				registry.mReader[token](reader);
			}
		}
	}
	return true;
}

bool EngineReceiver::hasLostConnection() const {
	return mNoDataCount > 300;
}

void EngineReceiver::clearLostConnection() {
	mNoDataCount = 0;
}


} // namespace ds
