#include "stdafx.h"

#include "single_udp_receiver.h"
#include <iostream>
#include <Poco/Net/NetException.h>
#include "ds/util/string_util.h"
#include <ds/debug/logger.h>

namespace {

class BadIpException : public std::exception {
public:
	BadIpException(const std::string &ip)	{
		mMsg = ip + " is outside of the Multicast range. Please choose an address between 224.0.0.0 and 239.255.255.255.";
	}

	const char *what() const throw() {
		return mMsg.c_str();
	}

private:
	std::string mMsg;
};

}

namespace ds
{

UdpReceiver::UdpReceiver(int numThreads)
	: mInitialized(false)
	, mReceiveBufferMaxSize(0)
	, mConnected(false)
{
}

UdpReceiver::~UdpReceiver(){
	close();
}

bool UdpReceiver::initialize(const std::string &ip, const std::string &portSz)
{
	/*std::vector<std::string> numbers = ds::split(ip, ".");
	int value;
	ds::string_to_value(numbers.front(), value);*/
	mConnected = false;
	mIp = ip;
	mPort = portSz;
	try	{
		unsigned short port;
		ds::string_to_value(portSz, port);
		mSocket.bind(Poco::Net::SocketAddress(ip, port));
		mSocket.setReceiveTimeout(0);

		mSocket.setReuseAddress(true);
		mSocket.setReusePort(true);
		//mSocket.connect(Poco::Net::SocketAddress(ip, port));
		mSocket.setBlocking(false);
		mSocket.setSendBufferSize(ds::NET_MAX_UDP_PACKET_SIZE);

		mReceiveBufferMaxSize = mSocket.getReceiveBufferSize();
		if(mReceiveBufferMaxSize <= 0){
			DS_LOG_WARNING("UdpConnection::initialize() Couldn't determine a receive buffer size");
		}
		if(!mReceiveBuffer.setSize(mReceiveBufferMaxSize)){
			DS_LOG_WARNING("UdpConnection::initialize() Can't allocate receive buffer");
		}

		mInitialized = true;
		return true;
	} catch(std::exception &e)
	{
		DS_LOG_WARNING("SingleUdpReceiver exception starting: " << e.what());
	} catch(...)
	{
		DS_LOG_WARNING("SingleUdpReceiver unknown exception starting");
	}

	return false;
}

bool UdpReceiver::connect(const std::string &ip, const std::string &portSz) {
	mIp = ip;
	mPort = portSz;
	mConnected = false;
	try {
		unsigned short port;
		ds::string_to_value(portSz, port);
		
		
		mSocket.bind(Poco::Net::SocketAddress(), true, true);
		mSocket.setReuseAddress(true);
		mSocket.setReusePort(true);
		mSocket.connect(Poco::Net::SocketAddress(ip, port));
		mSocket.setReceiveTimeout(0);
		mSocket.setBlocking(false);
		mSocket.setSendBufferSize(ds::NET_MAX_UDP_PACKET_SIZE);

		mReceiveBufferMaxSize = mSocket.getReceiveBufferSize();
		if(mReceiveBufferMaxSize <= 0) {
			DS_LOG_WARNING("UdpConnection::initialize() Couldn't determine a receive buffer size");
		}
		if(!mReceiveBuffer.setSize(mReceiveBufferMaxSize)) {
			DS_LOG_WARNING("UdpConnection::initialize() Can't allocate receive buffer");
		}

		mInitialized = true;
		mConnected = true;
		return true;
	} catch(std::exception &e)
	{
		DS_LOG_WARNING("SingleUdpReceiver exception connecting: " << e.what());
	} catch(...)
	{
		DS_LOG_WARNING("SingleUdpReceiver unknown exception connecting");
	}

	return false;
}

void UdpReceiver::close(){
	mInitialized = false;

	try
	{
		mSocket.close();
	} catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
}

void UdpReceiver::renew() {
	close();
	initialize(mIp, mPort);
}


bool UdpReceiver::sendMessage(const std::string &data){
	return sendMessage(data.c_str(), static_cast<int>(data.length()));
}


bool UdpReceiver::sendMessage(const char *data, int size) {
	int sentBytes = 0;
	try {
		sentBytes = mSocket.sendBytes(data, size);
	} catch(std::exception& e) {
		DS_LOG_WARNING("Exception sending bytes: " << e.what());
	}
	return  sentBytes > 0;
}

int UdpReceiver::recvMessage(std::string &msg){
	if(!mInitialized)
		return 0;

	try	{
		if(mSocket.available() <= 0) {
			return 0;
		}

		int size = mSocket.receiveBytes(mReceiveBuffer.data(), static_cast<int>(mReceiveBuffer.alloc()));
		if(size > 0) {
			msg.assign(mReceiveBuffer.data(), size);
		}
		return size;
	} catch(std::exception &e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}

bool UdpReceiver::canRecv() const{
	if(!mInitialized)
		return 0;

	try	{
		return mSocket.available() > 0;
	} catch(std::exception &e){
		std::cout << e.what() << std::endl;
	}

	return false;
}

bool UdpReceiver::initialized() const {
	return mInitialized;
}

}
