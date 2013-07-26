#include "ds/network/tcp_server.h"

#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

namespace ds {
namespace net {

namespace {
	class EchoConnection: public Poco::Net::TCPServerConnection {
	public:
		EchoConnection(	const Poco::Net::StreamSocket& s,
						std::shared_ptr<TcpServer::SendBucket>& b,
						std::shared_ptr<ds::AsyncQueue<std::string>>& rq,
						std::shared_ptr<bool>& st)
				: Poco::Net::TCPServerConnection(s)
				, mBucket(b)
				, mSendQueue(b->startQueue())
				, mReceiveQueue(rq)
				, mSt(st) {
		}

		~EchoConnection() {
			mBucket->endQueue(mSendQueue);
		}
		
		void run() {
			const Poco::Timespan			net_timeout(1 * 500000);
			Poco::Net::StreamSocket&		ss = socket();
			ss.setSendTimeout(net_timeout);
			ss.setReceiveTimeout(net_timeout);
			// Keep running until the connection is closed externally
			bool							keepRunning = true;
			while (keepRunning) {
				try {
					sendTo(ss);
					receiveFrom(ss);
				} catch (Poco::TimeoutException const&) {
					const bool				stopped = *mSt;
					if (stopped) keepRunning = false;
				} catch (std::exception const&) {
					keepRunning = false;
				}
			}
		}

	private:
		void		sendTo(Poco::Net::StreamSocket& socket) {
			if (!mSendQueue) return;
			const std::vector<std::string>*	send = mSendQueue->update();
			if (!send) return;

			for (auto it=send->begin(), end=send->end(); it!=end; ++it) {
				const std::string&	data(*it);
				if (!data.empty()) socket.sendBytes(data.data(), data.size());
			}
		}

		void		receiveFrom(Poco::Net::StreamSocket& socket) {
			const int		n = socket.receiveBytes(mBuffer, sizeof(mBuffer));
			if (n > 0) mReceiveQueue->push(std::string(mBuffer, n));
		}

		char										mBuffer[256];
		std::shared_ptr<TcpServer::SendBucket>		mBucket;
		ds::AsyncQueue<std::string>*				mSendQueue;
		std::shared_ptr<ds::AsyncQueue<std::string>>	mReceiveQueue;
		std::shared_ptr<bool>							mSt;
	};

	class ConnectionFactory: public Poco::Net::TCPServerConnectionFactory {
	public:
		ConnectionFactory(std::shared_ptr<TcpServer::SendBucket>& b, std::shared_ptr<ds::AsyncQueue<std::string>>& rq, std::shared_ptr<bool>& st)
				: mBucket(b)
				, mReceiveQueue(rq)
				, mSt(st) {
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) {
			return new EchoConnection(socket, mBucket, mReceiveQueue, mSt);
		}

		std::shared_ptr<TcpServer::SendBucket>			mBucket;
		std::shared_ptr<ds::AsyncQueue<std::string>>	mReceiveQueue;
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
		, mBucket(new SendBucket())
		, mReceiveQueue(new ds::AsyncQueue<std::string>())
		, mServer(new ConnectionFactory(mBucket, mReceiveQueue, mStopped), Poco::Net::ServerSocket(address)) {
	if (!mStopped) throw std::runtime_error("TcpServer failed making mStopped");
	if (!mBucket) throw std::runtime_error("TcpServer failed making mBucket");
	if (!mReceiveQueue) throw std::runtime_error("TcpServer failed making mReceiveQueue");

	try {
		mServer.start();
	} catch (std::exception const& ex) {
		DS_LOG_ERROR("TcpServer failed to start (" << address.toString() << ") error=" << ex.what());
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

void TcpServer::sendToClients(const std::string& data) {
	mBucket->add(data);
}

void TcpServer::update(const ds::UpdateParams&) {
	const std::vector<std::string>* vec = mReceiveQueue->update();
	if (!vec) return;

	for (auto it=mListener.begin(), end=mListener.end(); it != end; ++it) {
		for (auto pit=vec->begin(), pend=vec->end(); pit != pend; ++pit) (*it)(*pit);
	}
}

/**
 * \class ds::TcpServer::SendBucket
 */
TcpServer::SendBucket::SendBucket() {
}

ds::AsyncQueue<std::string>* TcpServer::SendBucket::startQueue() {
	try {
		ds::AsyncQueue<std::string>*	q = new ds::AsyncQueue<std::string>();
		if (!q) return nullptr;
		Poco::Mutex::ScopedLock			l(mMutex);
		mQueue.push_back(q);
		return q;
	} catch (std::exception const&) {
	}
	return nullptr;
}

void TcpServer::SendBucket::endQueue(ds::AsyncQueue<std::string>* q) {
	if (!q) return;

	Poco::Mutex::ScopedLock		l(mMutex);
	for (auto it=mQueue.begin(), end=mQueue.end(); it!=end; ++it) {
		if ((*it) == q) {
			mQueue.erase(it);
			return;
		}
	}
}

void TcpServer::SendBucket::add(const std::string& data) {
	if (data.empty()) return;

	Poco::Mutex::ScopedLock		l(mMutex);
	for (auto it=mQueue.begin(), end=mQueue.end(); it!=end; ++it) {
		auto*	q = *it;
		if (q) q->push(data);
	}
}

} // namespace net
} // namespace ds