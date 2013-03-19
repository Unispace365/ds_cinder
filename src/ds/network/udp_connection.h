#pragma once
#ifndef DS_NETWORK_UDPCONNECTION_H
#define DS_NETWORK_UDPCONNECTION_H

#include <memory>
#include <Poco/Net/MulticastSocket.h>
#include "ds/data/raw_data_buffer.h"
#include "ds/network/net_connection.h"

namespace ds
{

extern const unsigned int		NET_MAX_UDP_PACKET_SIZE;

class UdpConnection : public NetConnection
{
  public:
    UdpConnection(int numThreads = 1);
    ~UdpConnection();

    bool initialize(bool server, const std::string &ip, const std::string &port);
    void close();

    bool sendMessage(const std::string &data);
    bool sendMessage(const char *data, int size);

    int recvMessage(std::string &msg);

    bool isServer() const;

    bool initialized() const;

  private:
		Poco::Net::MulticastSocket	mSocket;
    bool                        mServer;
    bool                        mInitialized;
    int                         mReceiveBufferMaxSize;
    RawDataBuffer               mReceiveBuffer;
};

}

#endif//DS_NETWORK_UDPCONNECTION_H
