#include "stdafx.h"

#include "ds/app/engine/engine_io.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/debug/logger.h"
#include "ds/util/string_util.h"
#include "snappy.h"

namespace ds {

/**
 * \class ds::EngineSender
 */
EngineSender::EngineSender(ds::NetConnection& con)
		: mConnection(con) {
}

/**
 * \class ds::EngineSender::AutoSend
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

	const int size = mData.size();
	mSender.mRawDataBuffer.setSize(size);
	mData.readRaw(mSender.mRawDataBuffer.data(), size);
	snappy::Compress(mSender.mRawDataBuffer.data(), size, &mSender.mCompressionBuffer);
	mSender.mConnection.sendMessage(mSender.mCompressionBuffer);
	mData.clear();
}

/**
 * \class ds::EngineReceiver
 */
EngineReceiver::EngineReceiver(ds::NetConnection& con)
		: mConnection(con)
		, mHeaderId(0)
		, mCommandId(0)
		, mHeaderAndCommandOnly(false)
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
	return mReceiveBuffer;
}

bool EngineReceiver::receiveAndHandle(ds::BlobRegistry& registry, ds::BlobReader& reader) {
	EngineReceiver::AutoReceive   receive(*this);
	if (mReceiveBuffer.size() < 1) {
		++mNoDataCount;
		return false;
	}

	mNoDataCount = 0;

	const int					receiveSize = mReceiveBuffer.size();
	const char					size = static_cast<char>(registry.mReader.size());
	while (receive.mData.canRead<char>()) {
		const char				token = receive.mData.read<char>();
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
	return mNoDataCount > 100;
}

void EngineReceiver::clearLostConnection() {
	mNoDataCount = 0;
}

/**
 * \class ds::EngineReceiver::AutoReceive
 */
EngineReceiver::AutoReceive::AutoReceive(EngineReceiver& receiver)
		: mData(receiver.mReceiveBuffer) {
	mData.clear();
	if (receiver.mConnection.recvMessage(receiver.mCompressionBufferWrite)) {
		snappy::Uncompress(receiver.mCompressionBufferWrite.c_str(), receiver.mCompressionBufferWrite.size(), &receiver.mCompressionBufferRead);
		mData.addRaw(receiver.mCompressionBufferRead.c_str(), receiver.mCompressionBufferRead.size());
	}
}

} // namespace ds