#pragma once
#ifndef DS_NETWORK_TCPCLIENT_H_
#define DS_NETWORK_TCPCLIENT_H_

#include <functional>
#include <vector>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/Net/StreamSocket.h>
#include "ds/app/auto_update.h"
#include "ds/network/tcp_socket_sender.h"

namespace ds {
namespace net {

/**
 * \class ds::TcpClient
 * \brief Feed clients information about changes on a TCP socket.
 */
class TcpClient : public ds::AutoUpdate {
public:
	TcpClient(ds::ui::SpriteEngine&, const Poco::Net::SocketAddress&, const double poll_rate = 1.0);
	~TcpClient();

	void							add(const std::function<void(const std::string&)>&);

	// Send data to the server
	void							send(const std::string& data);

protected:
	// Flush any change notifications from the calling thread.
	virtual void					update(const ds::UpdateParams&);

private:
	class Loop : public Poco::Runnable {
	public:
		Poco::Mutex						mMutex;
		bool							mAbort;
		std::vector<std::string>		mUpdates;
		Poco::Net::StreamSocket			mSocket;

	public:
		Loop(const Poco::Net::SocketAddress&, const double poll_rate);

		virtual void					run();

	private:
		void							update(const std::string&);

		const Poco::Net::SocketAddress	mAddress;
		const double					mPollRate;
	};

	Poco::Thread					mThread;
	Loop							mLoop;
	std::vector<std::function<void(const std::string&)>>
									mListener;

	ds::net::TcpSocketSender		mSocketSender;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPCLIENT_H_