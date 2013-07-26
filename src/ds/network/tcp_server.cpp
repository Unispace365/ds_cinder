#include "ds/network/tcp_server.h"

#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

namespace ds {
namespace net {

namespace {
	class EchoConnection: public Poco::Net::TCPServerConnection {
	public:
		EchoConnection(	const Poco::Net::StreamSocket& s,
						std::shared_ptr<ds::AsyncQueue<std::string>>& q,
						std::shared_ptr<bool>& st)
				: Poco::Net::TCPServerConnection(s)
				, mQueue(q)
				, mSt(st) {
		}

		~EchoConnection() {
		}
		
		void run() {
			const Poco::Timespan			net_timeout(1 * 500000);
			Poco::Net::StreamSocket&		ss = socket();
			char							buffer[256];
			ss.setReceiveTimeout(net_timeout);
			// Keep running until the connection is closed externally
			bool							keepRunning = true;
			while (keepRunning) {
				try {
					const int				n = ss.receiveBytes(buffer, sizeof(buffer));
					if (n > 0) mQueue->push(std::string(buffer, n));
				} catch (Poco::TimeoutException const&) {
					const bool				stopped = *mSt;
					if (stopped) keepRunning = false;
				} catch (std::exception const&) {
					keepRunning = false;
				}
			}
		}

		std::shared_ptr<ds::AsyncQueue<std::string>>	mQueue;
		std::shared_ptr<bool>							mSt;
	};

	class ConnectionFactory: public Poco::Net::TCPServerConnectionFactory {
	public:
		ConnectionFactory(std::shared_ptr<ds::AsyncQueue<std::string>>& q, std::shared_ptr<bool>& st)
				: mQueue(q)
				, mSt(st) {
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) {
			return new EchoConnection(socket, mQueue, mSt);
		}

		std::shared_ptr<ds::AsyncQueue<std::string>>	mQueue;
		std::shared_ptr<bool>							mSt;
	};
}

/**
 * \class ds::TcpServer
 */
TcpServer::TcpServer(ds::ui::SpriteEngine& e, const Poco::Net::SocketAddress& address)
		: ds::AutoUpdate(e)
		, mAddress(address)
		, mStopped(new bool(false))
		, mQueue(new ds::AsyncQueue<std::string>())
		, mServer(new ConnectionFactory(mQueue, mStopped), Poco::Net::ServerSocket(address)) {
	if (!mStopped) throw std::runtime_error("TcpServer failed making mStopped");
	if (!mQueue) throw std::runtime_error("TcpServer failed making mQueue");

	try {
		mServer.start();
	} catch (std::exception const&) {
	}
}

TcpServer::~TcpServer() {
	(*mStopped) = true;

	try {
		mServer.stop();
	} catch (std::exception const&) {
	}
}

void TcpServer::add(const std::function<void(const std::string&)>& f) {
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

#if 0
// This is not how you'd do this --- if you want to send to the server,
// use a SocketSender like TcpClient
	// Send data through the server
	void							send(const std::string& data);

void TcpServer::send(const std::string& data) {
	if (data.empty()) return;
	try {
		Poco::Net::StreamSocket	socket(mAddress);
		socket.sendBytes(data.data(), data.size());
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("TcpServer::send() error sending data=" << data << " (" << ex.what() << ")");
		DS_DBG_CODE(std::cout << "TcpServer::send() error sending data=" << data << " (" << ex.what() << ")" << std::endl);
	}
}
#endif

void TcpServer::update(const ds::UpdateParams&) {
	const std::vector<std::string>* vec = mQueue->update();
	if (!vec) return;

	for (auto it=mListener.begin(), end=mListener.end(); it != end; ++it) {
		for (auto pit=vec->begin(), pend=vec->end(); pit != pend; ++pit) (*it)(*pit);
	}
}

} // namespace net
} // namespace ds