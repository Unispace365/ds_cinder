#pragma once
#ifndef DS_APP_ENGINEIO_H_
#define DS_APP_ENGINEIO_H_

#include "ds/data/data_buffer.h"
#include "ds/data/raw_data_buffer.h"
#include "ds/network/zmq_connection.h"

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
    EngineSender(ds::ZmqConnection&);

  private:
    ds::ZmqConnection&            mConnection;
    ds::DataBuffer                mSendBuffer;
    RawDataBuffer                 mRawDataBuffer;
    std::string                   mCompressionBuffer;

  public:
    class AutoSend {
      public:
        AutoSend(EngineSender&);
        ~AutoSend();

        ds::DataBuffer&           mData;

      private:
        EngineSender&             mSender;
    };
};

/**
 * \class ds::EngineReceiver
 * Receive data from a source.
 */
class EngineReceiver {
  public:
    EngineReceiver(ds::ZmqConnection&);

    ds::DataBuffer&               getData();
    // Convenience for clients with a blob reader, automatically
    // receive and handle the data.
    void                          receiveAndHandle(ds::BlobRegistry&, ds::BlobReader&);

  private:
    ds::ZmqConnection&            mConnection;
    ds::DataBuffer                mReceiveBuffer;
    std::string                   mCompressionBufferRead;
    std::string                   mCompressionBufferWrite;

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

#endif // DS_APP_ENGINEIO_H_