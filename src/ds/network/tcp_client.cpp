#include "ds/network/tcp_client.h"

#include <iostream>
#include <Poco/Net/NetException.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>

namespace ds {
namespace net {

/**
 * \class ds::TcpClient
 */
TcpClient::TcpClient(ds::ui::SpriteEngine& e, const Poco::Net::SocketAddress& address)
		: ds::AutoUpdate(e)
		, mLoop(address) {
	mSocketSender.addClient(address);
	try {
		mThread.start(mLoop);
	} catch (Poco::Exception&) {
//    std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
	} catch (std::exception&) {
//    std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
	}
}

TcpClient::~TcpClient() {
	{
		Poco::Mutex::ScopedLock		l(mLoop.mMutex);
		if (!mThread.isRunning()) return;
		mLoop.mAbort = true;
	}

	try {
		mThread.join();
	} catch (std::exception&) {
	}
}

void TcpClient::add(const std::function<void(const std::string&)>& f) {
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

void TcpClient::send(const std::string& data) {
	mSocketSender.send(data);
}

void TcpClient::update(const ds::UpdateParams&) {
	std::vector<std::string>	popped;
	{
		Poco::Mutex::ScopedLock	l(mLoop.mMutex);
		popped.swap(mLoop.mUpdates);
	}
	if (popped.empty()) return;

	for (auto it=mListener.begin(), end=mListener.end(); it != end; ++it) {
		for (auto pit=popped.begin(), pend=popped.end(); pit != pend; ++pit) (*it)(*pit);
	}
}

/**
 * \class ds::NodeWatcher::Loop
 */
TcpClient::Loop::Loop(const Poco::Net::SocketAddress& a)
		: mAbort(false)
		, mUpdates(0)
		, mAddress(a) {
}

void TcpClient::Loop::run() {
	static const int			BUF_SIZE = 4096;

	char						buf[BUF_SIZE];
	bool						needsConnect = true;
	const Poco::Timespan		connect_timeout(1 * 100000);

	while (true) {
		// Keep retrying the connection whenever it fails.
		if (needsConnect) {
			needsConnect = false;
			try {
				mSocket.connect(mAddress, connect_timeout);
				mSocket.setBlocking(false);
				mSocket.setReceiveTimeout(Poco::Timespan(10, 0));
			} catch (std::exception&) {
				needsConnect = true;
			}
		}

		int						length = 0;
		try {
			length = mSocket.receiveBytes(buf, BUF_SIZE);
			// I think a return of 0 here indicates that the connection is closed
		}
		catch (Poco::TimeoutException&) {
//			std::cout << "TcpWatcher timeout" << std::endl;
//			std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
		} catch (Poco::Net::InvalidSocketException&) {
			needsConnect = true;
		} catch (std::exception&) {
//			std::cout << "TcpWatcher exception " << ex.what() << std::endl;
//			std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
		}
		// Throttle this down, so I'm not slamming the mutex
		Poco::Thread::sleep(100);

		if ( length > 0 ) {
			update(std::string(buf, length));
		}

		{
			Poco::Mutex::ScopedLock	l(mMutex);
			if (mAbort) break;
		}
	}

	try {
//		mSocket.shutdown();
	} catch (std::exception&) {
	}	
}

void TcpClient::Loop::update(const std::string& str) {
	Poco::Mutex::ScopedLock	l(mMutex);
	try {
		mUpdates.push_back(str);
	} catch (std::exception&) {
	}
}

} // namespace net
} // namespace ds
