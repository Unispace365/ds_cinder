#pragma once
#ifndef DS_APP_ENGINE_ENGINEIO_H_
#define DS_APP_ENGINE_ENGINEIO_H_

#include "ds/data/data_buffer.h"
#include "ds/data/raw_data_buffer.h"
#include "ds/network/net_connection.h"

/**
 * Hide the busy work of sending information between the server and client.
 */

namespace ds {
class BlobReader;
class BlobRegistry;

/**
 * \class ds::EngineSender
 * Send data from a source to destination.
 */
class EngineSender {
public:
	EngineSender(ds::NetConnection&);

private:
	ds::NetConnection&			mConnection;
	ds::DataBuffer				mSendBuffer;
	RawDataBuffer				mRawDataBuffer;
	std::string					mCompressionBuffer;

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
 * \class ds::EngineReceiver
 * Receive data from a source.
 */
class EngineReceiver {
public:
	EngineReceiver(ds::NetConnection&);

	// A bit of a hack -- every state can be set to listen
	// only for the header and command, or everything. This
	// is used to stop me from receiving the entire world
	// when I'm not ready.
	void						setHeaderAndCommandIds(const char header, const char command);
	void						setHeaderAndCommandOnly(const bool = false);

	ds::DataBuffer&				getData();
	// Convenience for clients with a blob reader, automatically
	// receive and handle the data. Answer true if there was data.
	bool						receiveAndHandle(ds::BlobRegistry&, ds::BlobReader&);
	bool						hasLostConnection() const;
	void						clearLostConnection();

private:
	ds::NetConnection&			mConnection;
	ds::DataBuffer				mReceiveBuffer;
	std::string					mCompressionBufferRead;
	std::string					mCompressionBufferWrite;
	// The header and command blob IDs, used for filtering. The header
	// and command are always processed, but anything else depends on the state
	char						mHeaderId,
								mCommandId;
	bool						mHeaderAndCommandOnly;

	// Track when I try to receive but don't have any data. If this happens
	// enough, then my network connection has likely dropped.
	int							mNoDataCount;

public:
	// Clients can use this to handle a raw stream of
	// data from the source.
	class AutoReceive {
	public:
		AutoReceive(EngineReceiver&);
		ds::DataBuffer&           mData;
	};
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINEIO_H_