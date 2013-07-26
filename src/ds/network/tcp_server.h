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
	~TcpServer();

	void							add(const std::function<void(const std::string&)>&);

protected:
	// Flush any change notifications from the calling thread.
	virtual void					update(const ds::UpdateParams&);

private:
	const Poco::Net::SocketAddress	mAddress;
	// These shared_ptr objects are shared with each connection I create.
	// Connections might exist past the life of this class.
	std::shared_ptr<bool>			mStopped;
	std::shared_ptr<ds::AsyncQueue<std::string>>
									mQueue;
	Poco::Net::TCPServer			mServer;
	std::vector<std::function<void(const std::string&)>>
									mListener;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPSERVER_H_
