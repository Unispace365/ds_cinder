#include "ds/network/tcp_server.h"

#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

namespace ds {
namespace net {

namespace {
	class EchoConnection: public Poco::Net::TCPServerConnection {
	public:
		EchoConnection(	const Poco::Net::StreamSocket& s,
						ds::AsyncQueue<std::string>& q,
						TcpServer::ClientManager& cm)
				: Poco::Net::TCPServerConnection(s)
				, mQueue(q)
				, mClientManager(cm) {
		}
		
		void run() {
			Poco::Net::StreamSocket&		ss = socket();
			const Poco::Net::SocketAddress	peerAddress(ss.peerAddress());
			mClientManager.addClient(peerAddress);
			try {
				char buffer[256];
				int n = ss.receiveBytes(buffer, sizeof(buffer));
				while (n > 0) {
					mQueue.push(std::string(buffer, n));
//          std::cout << "handle size=" << n << " me=" << (void*)this << std::endl;
					n = ss.receiveBytes(buffer, sizeof(buffer));
				}
			}
			catch (Poco::Exception&) {
//				std::cerr << "EchoConnection: " << exc.displayText() << std::endl;
			}
			mClientManager.removeClient(peerAddress);
		}

		ds::AsyncQueue<std::string>&	mQueue;
		TcpServer::ClientManager&		mClientManager;
	};

	class ConnectionFactory: public Poco::Net::TCPServerConnectionFactory {
	public:
		ConnectionFactory(ds::AsyncQueue<std::string>& q, TcpServer::ClientManager& cm)
				: mQueue(q)
				, mClientManager(cm) {
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) {
			return new EchoConnection(socket, mQueue, mClientManager);
		}

		ds::AsyncQueue<std::string>&	mQueue;
		TcpServer::ClientManager&		mClientManager;
	};
}

/**
 * \class ds::TcpServer
 */
TcpServer::TcpServer(ds::ui::SpriteEngine& e, const Poco::Net::SocketAddress& address)
		: ds::AutoUpdate(e)
		, mAddress(address)
		, mServer(new ConnectionFactory(mQueue, mClientManager), Poco::Net::ServerSocket(address)) {
	try {
		mServer.start();
	} catch (std::exception const&) {
	}
}

void TcpServer::add(const std::function<void(const std::string&)>& f)
{
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

void TcpServer::sendToClients(const std::string& data)
{
	if (data.empty()) return;

	try {
		Poco::Net::StreamSocket		socket(mAddress);
		socket.sendBytes(data.data(), data.size());
	} catch (std::exception const& ex) {
		DS_LOG_WARNING("TcpServer::send() error sending data=" << data << " (" << ex.what() << ")");
		DS_DBG_CODE(std::cout << "TcpServer::send() error sending data=" << data << " (" << ex.what() << ")" << std::endl);
	}
}

void TcpServer::update(const ds::UpdateParams&)
{
	const std::vector<std::string>* vec = mQueue.update();
	if (!vec) return;

	for (auto it=mListener.begin(), end=mListener.end(); it != end; ++it) {
		for (auto pit=vec->begin(), pend=vec->end(); pit != pend; ++pit) (*it)(*pit);
	}
}

/**
 * \class ds::TcpServer::ClientManager
 */
TcpServer::ClientManager::ClientManager() {
}

void TcpServer::ClientManager::addClient(const Poco::Net::SocketAddress& a) {
	try {
		boost::lock_guard<boost::mutex> lock(mMutex);
		mClients.push_back(a);
	} catch (std::exception const&) {
	}
std::cout << "add, size=" << mClients.size() << std::endl;
}

void TcpServer::ClientManager::removeClient(const Poco::Net::SocketAddress& a) {
	try {
		boost::lock_guard<boost::mutex> lock(mMutex);
		for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
			Poco::Net::SocketAddress&	cmp(*it);
			if (cmp == a) {
				mClients.erase(it);
std::cout << "remove, size=" << mClients.size() << std::endl;
				return;
			}
		}
	} catch (std::exception const&) {
	}
std::cout << "no remove, size=" << mClients.size() << std::endl;
}

} // namespace net
} // namespace ds