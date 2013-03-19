#pragma once
#ifndef DS_NETWORK_NETCONNECTION_H
#define DS_NETWORK_NETCONNECTION_H

#include <string>

namespace ds
{

/**
 * ds::NetConnection
 * Abstract interface for a network connection.
 */
class NetConnection
{
  public:
    virtual ~NetConnection()      { }

    virtual bool initialize(bool server, const std::string &ip, const std::string &port) = 0;

    virtual bool sendMessage(const std::string &data) = 0;
    virtual bool sendMessage(const char *data, int size) = 0;

    virtual int recvMessage(std::string &msg) = 0;

    virtual bool isServer() const = 0;

    virtual bool initialized() const = 0;

  protected:
    NetConnection()               { }
};

}

#endif//DS_ZMQ_CONNECTION_H
