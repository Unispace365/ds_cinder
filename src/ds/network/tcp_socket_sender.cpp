#include "ds/network/tcp_socket_sender.h"

#include <Poco/Net/StreamSocket.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

namespace ds {
namespace net {

/**
 * \class ds::TcpSocketSender
 */
TcpSocketSender::TcpSocketSender() {
	try {
	    mThread.start(mWorker);
	} catch (std::exception const&) {
	}
}

TcpSocketSender::~TcpSocketSender() {
	mWorker.abort();
	try {
		mThread.join();
	} catch (std::exception const&) {
	}
}

void TcpSocketSender::addClient(const Poco::Net::SocketAddress& a) {
	mWorker.addClient(a);
}

void TcpSocketSender::removeClient(const Poco::Net::SocketAddress& a) {
	mWorker.removeClient(a);
}

void TcpSocketSender::send(const std::string& data) {
	mWorker.send(data);
}

/**
 * \class ds::TcpSocketSender::Worker
 */
TcpSocketSender::Worker::Worker() {
}

void TcpSocketSender::Worker::addClient(const Poco::Net::SocketAddress& a) {
	try {
		Poco::Mutex::ScopedLock		l(mMutex);
		mClients.push_back(a);
	} catch (std::exception const&) {
	}
}

void TcpSocketSender::Worker::removeClient(const Poco::Net::SocketAddress& a) {
	try {
		Poco::Mutex::ScopedLock		l(mMutex);
		for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
			Poco::Net::SocketAddress&	cmp(*it);
			if (cmp == a) {
				mClients.erase(it);
				return;
			}
		}
	} catch (std::exception const&) {
	}
}

void TcpSocketSender::Worker::send(const std::string& data) {
	if (data.empty()) return;

	Poco::Mutex::ScopedLock		l(mMutex);
	if (mAbort) return;

	try {
		mInput.push_back(data);
	} catch (std::exception const&) {
	}
	
	mCondition.signal();
}

void TcpSocketSender::Worker::abort() {
	Poco::Mutex::ScopedLock		l(mMutex);
	mAbort = true;
	mCondition.signal();
}

void TcpSocketSender::Worker::run() {
	// I want to stay locked for as little time as possible, so I pop off
	// the active inputs, then pop them back on as retired for reuse.
	std::vector<std::string>		ins;
	ins.reserve(16);

	while (true) {
		// Pop the inputs
		{
			Poco::Mutex::ScopedLock		l(mMutex);
			mInput.swap(ins);
		}

		// Perform each input
		perform(ins);
		ins.clear();

		{
			Poco::Mutex::ScopedLock		l(mMutex);
			if (mAbort) break;
		}

		// If more input came in during the time I've been
		// processing keep going, otherwise wait.
		mMutex.lock();
		if (!mAbort && mInput.size() < 1) mCondition.wait(mMutex);
		mMutex.unlock();
	}
}

void TcpSocketSender::Worker::perform(const std::vector<std::string>& data) {
	if (data.empty()) return;
	// Copy the list of addresses, so I don't block on them when sending
	std::vector<Poco::Net::SocketAddress>	vec;
	{
		Poco::Mutex::ScopedLock		l(mMutex);
		vec = mClients;
	}
	for (auto it=vec.begin(), end=vec.end(); it!=end; ++it) {
		try {
			Poco::Net::StreamSocket	socket(*it);
			for (auto dit = data.begin(), dend=data.end(); dit!=dend; ++dit) {
				const std::string&		d(*dit);
				socket.sendBytes(d.data(), d.size());
			}
		} catch (std::exception const& ex) {
			DS_LOG_WARNING("TcpServer::send() error sending data");
			DS_DBG_CODE(std::cout << "TcpServer::send() error sending data (" << ex.what() << ")" << std::endl);
		}
	}
}

} // namespace net
} // namespace ds