#include "stdafx.h"

#include "udp_connection.h"
#include <iostream>
#include <Poco/Net/NetException.h>
#include "ds\util\string_util.h"
#include <ds/debug/logger.h>

const unsigned int		ds::NET_MAX_UDP_PACKET_SIZE = 2000000;

namespace {

class BadIpException : public std::exception {
public:
	BadIpException(const std::string &ip){
		mMsg = ip + " is outside of the Multicast range. Please choose an address between 224.0.0.0 and 239.255.255.255.";
	}

	const char *what() const {
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};

}

namespace ds {

UdpConnection::UdpConnection(int numThreads)
	: mServer(false)
	, mInitialized(false)
	, mReceiveBufferMaxSize(0)
	, mReccBytes(0)
	, mSentBytes(0)
{
}

UdpConnection::~UdpConnection(){
	close();
}

bool UdpConnection::initialize(bool server, const std::string &ip, const std::string &portSz){
	DS_LOG_INFO("Starting udp connection at IP=" << ip << " port=" << portSz << " server=" << server);
	std::vector<std::string> numbers = ds::split(ip, ".");
	int value;
	ds::string_to_value(numbers.front(), value);
	if(!(value >= 224 && value <= 239)){
		DS_LOG_WARNING("Ip address must be in the multicast range, starting with 224 to 239, for example 239.255.42.50");
		return false;
	}

	mInitialized = false;
	mReccBytes = 0;
	mSentBytes = 0;
	mServer = server;
	mIp = ip;
	mPort = portSz;

	try {
		unsigned short port;
		ds::string_to_value(portSz, port);
		if(mServer)		{
			Poco::Net::SocketAddress sa = Poco::Net::SocketAddress(ip, port);
			mSocket.close();
			mSocket = Poco::Net::MulticastSocket();
			mSocket.connect(sa);
			mSocket.setReuseAddress(true);
			mSocket.setReusePort(true);
			mSocket.setBlocking(false);
			mSocket.setSendBufferSize(ds::NET_MAX_UDP_PACKET_SIZE);
		} else {
			Poco::Net::SocketAddress sa = Poco::Net::SocketAddress(ip, port);
			mSocket.close();
			mSocket = Poco::Net::MulticastSocket(Poco::Net::SocketAddress(Poco::Net::IPAddress(), sa.port()), true);
			
			mSocket.joinGroup(sa.host());
			mSocket.setReuseAddress(true);
			mSocket.setReusePort(true);
			mSocket.setBlocking(false);
			mSocket.setReceiveBufferSize(ds::NET_MAX_UDP_PACKET_SIZE);
			mSocket.setReceiveTimeout(1000);

			mReceiveBufferMaxSize = mSocket.getReceiveBufferSize();
			if(mReceiveBufferMaxSize <= 0){
				DS_LOG_WARNING("UdpConnection::initialize() Couldn't determine a receive buffer size");
			}
			if(!mReceiveBuffer.setSize(mReceiveBufferMaxSize)){
				DS_LOG_WARNING("UdpConnection::initialize() Can't allocate receive buffer");
			}
		}

		mInitialized = true;
		return true;
	} catch(Poco::Net::NetException& ne){
		DS_LOG_WARNING("Udp connection start Poco Net exception: " << ne.message());
	} catch(std::exception &e) {
		DS_LOG_WARNING("Exception starting a UDP connection: " << e.what());
	} catch(...) {
		DS_LOG_WARNING("UdpConnection Caught unknown exception");
	}

	mInitialized = false;
	return false;
}

void UdpConnection::close(){
	mInitialized = false;
	mServer = false;

	try{
		mSocket.close();
	} catch(std::exception &e) {
		DS_LOG_WARNING("Exception closing socket in UdpConnection: " << e.what());
	}
}

void UdpConnection::renew() {
	const bool		server = mServer;
	//close();
	initialize(server, mIp, mPort);
}


bool UdpConnection::sendMessage(const std::string &data){
	if(!mInitialized)
		return false;

	return sendMessage(data.c_str(), static_cast<int>(data.size()));
}

bool UdpConnection::sendMessage(const char *data, int size){
	if(!mInitialized || size < 1){
		return false;
	}

	try	{
		const int sentAmt = mSocket.sendBytes(data, size);
		mSentBytes += sentAmt;
		return sentAmt > 0;
	} catch(Poco::Net::NetException &e)	{
		DS_LOG_WARNING( "UdpConnection::sendMessage() error " << e.message());
	} catch(std::exception &e)	{
		DS_LOG_WARNING("UdpConnection::sendMessage() std::exception: " << e.what());
	}

	return false;
}

int UdpConnection::recvMessage(std::string &msg){
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
		mReccBytes += size;
		return size;
	} catch(Poco::Net::NetException &e)	{
		DS_LOG_WARNING("UdpConnection::recvMessage() error " << e.message());
	} catch(std::exception &e)	{
		DS_LOG_WARNING("UdpConnection::recvMessage() std::exception: " << e.what());
	}

	return 0;
}

bool UdpConnection::canRecv() const{
	if(!mInitialized)
		return 0;

	try{
		return mSocket.available() > 0;
	} catch(Poco::Net::NetException &e)	{
		DS_LOG_WARNING("UdpConnection::canRecv() error " << e.message());
	} catch(std::exception &e)	{
		DS_LOG_WARNING("UdpConnection::canRecv() std::exception: " << e.what());
	}

	return false;
}

bool UdpConnection::isServer() const{
	return mServer;
}

bool UdpConnection::initialized() const{
	return mInitialized;
}

int UdpConnection::getReceivedBytes(){
	int returnAmount = mReccBytes;
	mReccBytes = 0;
	return returnAmount;
}

int UdpConnection::getSentBytes(){
	int returnAmount = mSentBytes;
	mSentBytes = 0;
	return returnAmount;
}

}
