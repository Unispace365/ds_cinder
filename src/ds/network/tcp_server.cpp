#include "stdafx.h"

#include "ds/network/tcp_server.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "tcp_client.h"

namespace ds {
namespace net {

namespace {
	class EchoConnection: public Poco::Net::TCPServerConnection {
	public:
		EchoConnection(	const Poco::Net::StreamSocket& s,
						std::shared_ptr<TcpServer::SendBucket>& b,
						std::shared_ptr<ds::AsyncQueue<std::string>>& rq,
						std::shared_ptr<bool>& st,
						const std::string& wakeup,
						const std::string &terminator)
				: Poco::Net::TCPServerConnection(s)
				, mBucket(b)
				, mSendQueue(b->startQueue())
				, mReceiveQueue(rq)
				, mSt(st)
				, mTerminator(terminator) {
			if (mSendQueue && !wakeup.empty()) mSendQueue->push(wakeup);
		}

		~EchoConnection() {
			mBucket->endQueue(mSendQueue);
		}
		
		void run() {
			Poco::Net::StreamSocket&		ss = socket();
			ss.setNoDelay(true);
			ss.setBlocking(false);
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
				if (keepRunning) {
					Poco::Thread::sleep(1);
				}
			}
		}

	private:
		void		sendTo(Poco::Net::StreamSocket& socket) {
			if (!mSendQueue) return;
			const std::vector<std::string>*	send = mSendQueue->update();
			if (!send) return;

			// Compile into a single chunk of data. Note that I've seen
			// messages not go through when we try to make a series
			// of sendBytes() calls, so this prevents that.
			std::stringstream				buf;
			for (const auto& d : *send) {
				if (!d.empty()) buf << d << mTerminator;
			}

			TcpClient::sendBytes(socket, buf.str());
		}

		void		receiveFrom(Poco::Net::StreamSocket& socket) {
			int					n = 0;
			while ((n = socket.receiveBytes(mBuffer, sizeof(mBuffer))) > 0) {
				const std::string	incoming(mBuffer, n);
				if (mTerminator.empty()) {
					mReceiveQueue->push(incoming);
				} else {
					mWaiting += incoming;
					std::vector<std::string> all;
					boost::split(all, mWaiting, boost::is_any_of(mTerminator));
					mWaiting.clear();
					// The last element will be an empty string if this update() str ended
					// with the terminator; if it's not, then it's a partial, so track that.
					if (!all.empty() && !all.back().empty()) {
						mWaiting = all.back();
						all.pop_back();
					}
					for (auto it=all.begin(), end=all.end(); it!=end; ++it) {
						if (it->empty()) continue;
						mReceiveQueue->push(*it);
					}
				}
			}
		}

		static const int								BUFFER_SIZE = 4096;
		char											mBuffer[BUFFER_SIZE];
		std::shared_ptr<TcpServer::SendBucket>			mBucket;
		ds::AsyncQueue<std::string>*					mSendQueue;
		std::shared_ptr<ds::AsyncQueue<std::string>>	mReceiveQueue;
		std::shared_ptr<bool>							mSt;
		const std::string								mTerminator;
		std::string										mWaiting;
	};

	class ConnectionFactory: public Poco::Net::TCPServerConnectionFactory {
	public:
		ConnectionFactory(	std::shared_ptr<TcpServer::SendBucket>& b, std::shared_ptr<ds::AsyncQueue<std::string>>& rq,
							std::shared_ptr<bool>& st, const std::string& wakeup, const std::string &terminator)
				: mBucket(b)
				, mReceiveQueue(rq)
				, mSt(st)
				, mWakeup(wakeup)
				, mTerminator(terminator) {
		}

		Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) {
			return new EchoConnection(socket, mBucket, mReceiveQueue, mSt, mWakeup, mTerminator);
		}

		std::shared_ptr<TcpServer::SendBucket>			mBucket;
		std::shared_ptr<ds::AsyncQueue<std::string>>	mReceiveQueue;
		std::shared_ptr<bool>							mSt;
		const std::string								mWakeup;
		const std::string								mTerminator;
	};
}

/**
 * \class ds::TcpServer
 */
TcpServer::TcpServer(	ds::ui::SpriteEngine& e, const Poco::Net::SocketAddress& address,
						const std::string& wakeup, const std::string &terminator)
		: ds::AutoUpdate(e)
		, mAddress(address)
		, mStopped(new bool(false))
		, mBucket(new SendBucket())
		, mReceiveQueue(new ds::AsyncQueue<std::string>())
		, mServer(new ConnectionFactory(mBucket, mReceiveQueue, mStopped, wakeup, terminator), Poco::Net::ServerSocket(address)) {
	if (!mStopped) DS_LOG_WARNING("TcpServer failed making mStopped");
	if(!mBucket) DS_LOG_WARNING("TcpServer failed making mBucket");
	if(!mReceiveQueue) DS_LOG_WARNING("TcpServer failed making mReceiveQueue");

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