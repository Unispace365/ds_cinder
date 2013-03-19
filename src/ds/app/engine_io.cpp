#include "ds/app/engine_io.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/debug/logger.h"
#include "ds/util/string_util.h"
#include "snappy.h"

namespace ds {

/**
 * \class ds::EngineSender
 */
EngineSender::EngineSender(ds::ZmqConnection& con)
    : mConnection(con)
{
}

/**
 * \class ds::EngineSender::AutoSend
 */
EngineSender::AutoSend::AutoSend(EngineSender& sender)
  : mData(sender.mSendBuffer)
  , mSender(sender)
{
  mData.clear();
}

EngineSender::AutoSend::~AutoSend()
{
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
EngineReceiver::EngineReceiver(ds::ZmqConnection& con)
    : mConnection(con)
{
}

ds::DataBuffer& EngineReceiver::getData()
{
  return mReceiveBuffer;
}

void EngineReceiver::receiveAndHandle(ds::BlobRegistry& registry, ds::BlobReader& reader)
{
  EngineReceiver::AutoReceive   receive(*this);
  const char                    size = static_cast<char>(registry.mReader.size());
  while (receive.mData.canRead<char>()) {
    const char  token = receive.mData.read<char>();
    if (token > 0 && token < size) registry.mReader[token](reader);
  }
}

/**
 * \class ds::EngineReceiver::AutoReceive
 */
EngineReceiver::AutoReceive::AutoReceive(EngineReceiver& receiver)
  : mData(receiver.mReceiveBuffer)
{
  mData.clear();
  if (receiver.mConnection.recvMessage(receiver.mCompressionBufferWrite)) {
    snappy::Uncompress(receiver.mCompressionBufferWrite.c_str(), receiver.mCompressionBufferWrite.size(), &receiver.mCompressionBufferRead);
    mData.addRaw(receiver.mCompressionBufferRead.c_str(), receiver.mCompressionBufferRead.size());
  }
}

} // namespace ds
