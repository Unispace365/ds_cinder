#pragma once
#ifndef DS_NETWORK_TCPSERVER_H_
#define DS_NETWORK_TCPSERVER_H_

#include <Poco/Net/TCPServer.h>
#include <cinder/Thread.h>
#include "ds/app/auto_update.h"
#include "ds/thread/async_queue.h"

namespace ds {
namespace net {

/**
 * \class ds::net::TcpServer
 * \brief Start a server that outside clients can connect to, and will report any changes
 * to the calling application.
 */
class TcpServer : public ds::AutoUpdate {
public:
	TcpServer(ds::ui::SpriteEngine&, const Poco::Net::SocketAddress&);

	void							add(const std::function<void(const std::string&)>&);
	// Send data through the server
	void							sendToClients(const std::string& data);

protected:
	// Flush any change notifications from the calling thread.
	virtual void					update(const ds::UpdateParams&);

private:
	const Poco::Net::SocketAddress	mAddress;
	ds::AsyncQueue<std::string>		mQueue;
	Poco::Net::TCPServer			mServer;
	std::vector<std::function<void(const std::string&)>>
									mListener;

	// Keep track of clients as they exist.
public:
	class ClientManager {
	public:
		ClientManager();

		void						addClient(const Poco::Net::SocketAddress&);
		void						removeClient(const Poco::Net::SocketAddress&);
		void						send(const std::string& data);

	private:
		std::mutex					mMutex;
		std::vector<Poco::Net::SocketAddress>
									mClients;
	};
private:
	// Currently sending data synchronously, but really really need to put this in a thread.
	ClientManager					mClientManager;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPSERVER_H_
