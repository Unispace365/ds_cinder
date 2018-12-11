#pragma once
#ifndef DS_APP_ENGINE_ENGINEIO_H_
#define DS_APP_ENGINE_ENGINEIO_H_

#include "ds/data/data_buffer.h"
#include "ds/query/recycle_array.h"
#include "ds/network/net_connection.h"
#include "ds/network/packet_chunker.h"

/**
 * Hide the busy work of sending information between the server and client.
 */

namespace ds {
class BlobReader;
class BlobRegistry;

/**
 * \class EngineSender
 * Send data from a source to destination.
 */
class EngineSender {
public:
	EngineSender(ds::NetConnection&, const bool useChunker);

	void						setPacketNumber(unsigned int packetId);

private:
	ds::NetConnection&			mConnection;
	ds::DataBuffer				mSendBuffer;
	RecycleArray<char>			mRawDataBuffer;
	std::string					mCompressionBuffer;
	unsigned int				mPacketId;
	bool						mUseChunker;

public:
	class AutoSend {
	public:
		AutoSend(EngineSender&);
		~AutoSend();

		ds::DataBuffer&			mData;

	private:
		EngineSender&			mSender;
	};
};

/**
 * \class EngineReceiver
 * Receive data from a source.
 */
class EngineReceiver {
public:
	EngineReceiver(ds::NetConnection&, const bool useChunker);

	/// A bit of a hack -- every state can be set to listen
	/// only for the header and command, or everything. This
	/// is used to stop me from receiving the entire world
	/// when I'm not ready.
	void						setHeaderAndCommandIds(const char header, const char command);
	void						setHeaderAndCommandOnly(const bool = false);

	ds::DataBuffer&				getData();
	/// Convenience for clients with a blob reader, automatically
	/// receive and handle the data. Answer true if there was data.
	bool						receiveBlob();
	bool						handleBlob(ds::BlobRegistry&, ds::BlobReader&, bool& morePacketsAvailable);
	bool						hasLostConnection() const;
	void						clearLostConnection();

private:
	ds::DataBuffer				mCurrentDataBuffer;
	ds::NetConnection&			mConnection;
	std::string					mCompressionBufferRead;
	std::string					mCompressionBufferWrite;
	/// The header and command blob IDs, used for filtering. The header
	/// and command are always processed, but anything else depends on the state
	char						mHeaderId,
								mCommandId;
	bool						mHeaderAndCommandOnly;

	/// Track when I try to receive but don't have any data. If this happens
	/// enough, then my network connection has likely dropped.
	int							mNoDataCount;

	/// Keep track of all the packets we receive.
	/// This is in case we're running slower than the server,
	/// in which case we can run through and update all the buffers at once and catch up
	std::vector<std::string>	mReceiveBuffers;
	ds::net::DeChunker			mDechunker;
	bool						mUseChunker;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINEIO_H_
