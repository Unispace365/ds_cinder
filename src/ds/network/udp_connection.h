#pragma once
#ifndef DS_NETWORK_UDPCONNECTION_H
#define DS_NETWORK_UDPCONNECTION_H

#include <memory>
#include <Poco/Net/MulticastSocket.h>
#include "ds/query/recycle_array.h"
#include "ds/network/net_connection.h"

namespace ds
{

extern const unsigned int		NET_MAX_UDP_PACKET_SIZE;

class UdpConnection : public NetConnection {
public:
	UdpConnection(int numThreads = 1);
	~UdpConnection();

	bool initialize(bool server, const std::string &ip, const std::string &port);
	void close();
	// Convenience to close and reinitialize
	void renew();

	bool sendMessage(const std::string &data);
	bool sendMessage(const char *data, int size);

	int recvMessage(std::string &msg);
	// Answer true if I have more data to receive, false otherwise.
	bool canRecv() const;

	bool isServer() const;

	bool initialized() const;

	// For status, will return the number of bytes sent/rec since the last time this was called
	int getReceivedBytes();
	int getSentBytes();

private:
	Poco::Net::MulticastSocket	mSocket;
	int							mSentBytes;
	int							mReccBytes;
	bool						mInitialized;
	int							mReceiveBufferMaxSize;
	RecycleArray<char>			mReceiveBuffer;
	// Initialization value
	bool						mServer;
	std::string					mIp;
	std::string					mPort;
};

}

#endif//DS_NETWORK_UDPCONNECTION_H
