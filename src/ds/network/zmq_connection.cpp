#include "zmq_connection.h"
#include <iostream>
#include "ds\util\string_util.h"

namespace {

class BadIpException: public std::exception
{
  public:
    BadIpException(const std::string &ip)
    {
      mMsg = ip + " is in the Multicast range. Please choose a different ip; usually, \"localhost,\" 127.0.0.1, or your local ip.";
    }
    const char *what() const
    {
      return mMsg.c_str();
    }
  private:
    std::string mMsg;
};

}

namespace ds
{

ZmqConnection::ZmqConnection(int numThreads)
  : mContext(numThreads)
  , mServer(false)
  , mInitialized(false)
{
}

ZmqConnection::~ZmqConnection()
{
  close();
}

bool ZmqConnection::initialize( bool server, const std::string &ip, const std::string &port )
{
  std::vector<std::string> numbers = ds::split(ip, ".");
  int value;
  ds::string_to_value(numbers.front(), value);
  if (value >= 224 && value <= 239 )
    throw BadIpException(ip);

  mServer = server;
  mInitialized = true;
  try
  {
    if ( mServer )
    {
      mSocket = std::move( std::unique_ptr<zmq::socket_t>(new zmq::socket_t(mContext, ZMQ_DEALER)) );
      if ( mSocket ) {
        mSocket->bind(("tcp://"+ip+":"+port).c_str());
        int lingerVal = 0;
        mSocket->setsockopt(ZMQ_LINGER, &lingerVal, sizeof(int));
      }
    }
    else
    {
      mSocket = std::move( std::unique_ptr<zmq::socket_t>(new zmq::socket_t(mContext, ZMQ_DEALER)) );
      if ( mSocket ) {
        mSocket->connect(("tcp://"+ip+":"+port).c_str());
        int lingerVal = 0;
        mSocket->setsockopt(ZMQ_LINGER, &lingerVal, sizeof(int));
      }
    }

    return true;
  }
  catch ( zmq::error_t &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch ( std::exception &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Caught unknown exception" << std::endl;
  }

  return false;
}

void ZmqConnection::close()
{
  mInitialized = false;
  mServer = false;

  try
  {
    if ( mSocket )
    {
      mSocket->close();
      mSocket.release();
    }
  }
  catch ( zmq::error_t &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch ( std::exception &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Caught unknown exception" << std::endl;
  }
}

bool ZmqConnection::sendMessage( const std::string &data )
{
  if ( !mInitialized || !mSocket )
    return false;

  return sendMessage(data.c_str(), data.size());
}

bool ZmqConnection::sendMessage( const char *data, int size )
{
  if ( !mInitialized || !mSocket )
    return false;

  if ( mMsgSend.size() != size )
    mMsgSend.rebuild(size);
  memcpy(mMsgSend.data(), data, size);

  try
  {
    bool succeded = mSocket->send(mMsgSend, ZMQ_NOBLOCK);
    return succeded;
  }
  catch ( zmq::error_t &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch ( std::exception &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Caught unknown exception" << std::endl;
  }

  return false;
}

int ZmqConnection::recvMessage( std::string &msg )
{
  if ( !mInitialized || !mSocket )
    return 0;

  try
  {
    bool succeded = mSocket->recv(&mMsgRecv, ZMQ_NOBLOCK);

    msg.clear();
    if ( succeded )
      msg.assign(static_cast<char *>(mMsgRecv.data()), mMsgRecv.size());

    return succeded;
  }
  catch ( zmq::error_t &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch ( std::exception &e )
  {
    std::cout << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Caught unknown exception" << std::endl;
  }

  return false;
}

bool ZmqConnection::isServer() const
{
  return mServer;
}

bool ZmqConnection::initialized() const
{
  return mInitialized;
}

}
