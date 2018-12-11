#pragma once
#ifndef DS_NETWORK_TCPSOCKETSENDER_H_
#define DS_NETWORK_TCPSOCKETSENDER_H_

#include <vector>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/Net/SocketAddress.h>

namespace ds {
namespace net {

/**
 * \class TcpSocketSender
 * \brief Run a thread where I can send commands to a server.
 */
class TcpSocketSender {
public:
	TcpSocketSender();
	~TcpSocketSender();

	void							addClient(const Poco::Net::SocketAddress&);
	void							removeClient(const Poco::Net::SocketAddress&);
	void							send(const std::string& data);

private:
	class Worker : public Poco::Runnable {
	public:
		Worker();

		void						addClient(const Poco::Net::SocketAddress&);
		void						removeClient(const Poco::Net::SocketAddress&);
		void						send(const std::string& data);
		void						abort();

		virtual void				run();

	private:
		void						perform(const std::vector<std::string>&);

		Poco::Mutex					mMutex;
		Poco::Condition				mCondition;
		bool						mAbort;
		std::vector<std::string>	mInput;
		std::vector<Poco::Net::SocketAddress>
									mClients;
	};

	Poco::Thread					mThread;
	Worker							mWorker;
};

} // namespace net
} // namespace ds

#endif // DS_NETWORK_TCPSOCKETSENDER_H_
