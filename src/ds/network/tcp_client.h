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

namespace ds {
namespace net {

/**
 * \class ds::TcpClient
 * \brief Feed clients information about changes on a TCP socket.
 */
class TcpClient {
public:
	TcpClient(const Poco::Net::SocketAddress&);
	~TcpClient();

	void							add(const std::function<void(const std::string&)>&);

	// Until we have a generic messaging system I'm stuck requiring clients to poll.
	// This flushes any change notifications from the calling thread.
	void							update();

private:
	class Loop : public Poco::Runnable {
	public:
		Poco::Mutex					mMutex;
		bool						mAbort;
		std::vector<std::string>	mUpdates;
		Poco::Net::StreamSocket		mSocket;

	public:
		Loop();

		virtual void				run();

	private:
		void						update(const std::string&);
	};

	Poco::Thread					mThread;
	Loop							mLoop;
	std::vector<std::function<void(const std::string&)>>
									mListener;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPCLIENT_H_