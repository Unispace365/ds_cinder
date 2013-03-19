#pragma once
#ifndef DS_ZMQ_CONNECTION_H
#define DS_ZMQ_CONNECTION_H

#include "zmq.hpp"
#include <memory>
#include "ds/network/net_connection.h"

namespace ds
{

class ZmqConnection : public NetConnection
{
  public:
    ZmqConnection(int numThreads = 1);
    ~ZmqConnection();

    bool initialize(bool server, const std::string &ip, const std::string &port);
    void close();

    bool sendMessage(const std::string &data);
    bool sendMessage(const char *data, int size);

    int recvMessage(std::string &msg);

    bool isServer() const;

    bool initialized() const;
  private:
    zmq::context_t mContext;
    std::unique_ptr<zmq::socket_t> mSocket;

    zmq::message_t mMsgSend;
    zmq::message_t mMsgRecv;

    bool           mServer;
    bool           mInitialized;
};

}

#endif//DS_ZMQ_CONNECTION_H
