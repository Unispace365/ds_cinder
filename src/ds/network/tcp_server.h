#pragma once
#ifndef DS_NETWORK_TCPSERVER_H_
#define DS_NETWORK_TCPSERVER_H_

#include <Poco/Net/TCPServer.h>
#include "ds/thread/async_queue.h"

namespace ds {
namespace net {

/**
 * \class ds::net::TcpServer
 * \brief Start a server that outside clients can connect to, and will report any changes
 * to the calling application.
 */
class TcpServer {
public:
	TcpServer(const Poco::Net::SocketAddress&);

	void						add(const std::function<void(const std::string&)>&);
	void						update();
	// Send data through the server
	void						send(const std::string& data);

private:
	const Poco::Net::SocketAddress
								mAddress;
	ds::AsyncQueue<std::string> mQueue;
	Poco::Net::TCPServer        mServer;
	std::vector<std::function<void(const std::string&)>>
								mListener;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPSERVER_H_
