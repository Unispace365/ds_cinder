#pragma once
#ifndef DS_NETWORK_SINGLE_UDP_RECEIVER_H
#define DS_NETWORK_SINGLE_UDP_RECEIVER_H

#include <memory>
#include <Poco/Net/MulticastSocket.h>
#include "ds/query/recycle_array.h"
#include "ds/network/net_connection.h"

namespace ds
{

extern const unsigned int		NET_MAX_UDP_PACKET_SIZE;

class UdpReceiver : public NetConnection
{
public:
	UdpReceiver(int numThreads = 1);
	~UdpReceiver();

	bool initialize(bool server, const std::string &ip, const std::string &port);
	void close();
	// Convenience to close and reinitialize
	void renew();

	bool sendMessage(const std::string &data);
	bool sendMessage(const char *data, int size);

	bool isServer() const;

	int recvMessage(std::string &msg);
	// Answer true if I have more data to receive, false otherwise.
	bool canRecv() const;

	bool initialized() const;

private:
	Poco::Net::DatagramSocket	mSocket;
	bool						mInitialized;
	int							mReceiveBufferMaxSize;
	RecycleArray<char>			mReceiveBuffer;
	// Initialization value
	std::string					mIp;
	std::string					mPort;
};

}

#endif//DS_NETWORK_SINGLE_UDP_RECEIVER_H
