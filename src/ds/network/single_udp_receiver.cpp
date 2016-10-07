#include "single_udp_receiver.h"
#include <iostream>
#include <Poco/Net/NetException.h>
#include "ds\util\string_util.h"
#include <ds/debug/logger.h>

namespace {

	class BadIpException : public std::exception
	{
	public:
		BadIpException(const std::string &ip)
		{
			mMsg = ip + " is outside of the Multicast range. Please choose an address between 224.0.0.0 and 239.255.255.255.";
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

	UdpReceiver::UdpReceiver(int numThreads)
		: mInitialized(false)
		, mReceiveBufferMaxSize(0)
	{
	}

	UdpReceiver::~UdpReceiver()
	{
		close();
	}

	bool UdpReceiver::initialize(bool server, const std::string &ip, const std::string &portSz)
	{
		/*std::vector<std::string> numbers = ds::split(ip, ".");
		int value;
		ds::string_to_value(numbers.front(), value);*/

		mIp = ip;
		mPort = portSz;
		try
		{
			unsigned short        port;
			ds::string_to_value(portSz, port);
			mSocket.bind(Poco::Net::SocketAddress(ip, port));
			mSocket.setReceiveTimeout(0);

			mSocket.setReuseAddress(true);
			mSocket.setReusePort(true);
			//mSocket.connect(Poco::Net::SocketAddress(ip, port));
			mSocket.setBlocking(false);
			mSocket.setSendBufferSize(ds::NET_MAX_UDP_PACKET_SIZE);

			mReceiveBufferMaxSize = mSocket.getReceiveBufferSize();
			if (mReceiveBufferMaxSize <= 0){
				//throw std::exception("UdpConnection::initialize() Couldn't determine a receive buffer size");
				DS_LOG_WARNING("UdpConnection::initialize() Couldn't determine a receive buffer size");
			}
			if (!mReceiveBuffer.setSize(mReceiveBufferMaxSize)){
				//throw std::exception("UdpConnection::initialize() Can't allocate receive buffer");
				DS_LOG_WARNING("UdpConnection::initialize() Can't allocate receive buffer");
			}

			mInitialized = true;
			return true;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "Caught unknown exception" << std::endl;
		}

		return false;
	}

	void UdpReceiver::close()
	{
		mInitialized = false;

		try
		{
			mSocket.close();
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void UdpReceiver::renew() {
		close();
		initialize(true, mIp, mPort);
	}


	bool UdpReceiver::sendMessage(const std::string &data)
	{
		return false;
	}


	bool UdpReceiver::sendMessage(const char *data, int size)
	{
		return false;
	}


	bool UdpReceiver::isServer() const
	{
		return false;
	}

	int UdpReceiver::recvMessage(std::string &msg)
	{
		if (!mInitialized)
			return 0;

		try
		{
			if (mSocket.available() <= 0) {
				return 0;
			}

			int size = mSocket.receiveBytes(mReceiveBuffer.data(), mReceiveBuffer.alloc());
			if (size > 0) {
				msg.assign(mReceiveBuffer.data(), size);
			}
			return size;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}

		return 0;
	}

	bool UdpReceiver::canRecv() const
	{
		if (!mInitialized)
			return 0;

		try
		{
			return mSocket.available() > 0;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}

		return false;
	}

	bool UdpReceiver::initialized() const
	{
		return mInitialized;
	}

}
