#include "ds/network/tcp_client.h"

#include <Poco/Net/NetException.h>
#include <iostream>

namespace ds {
namespace net {

/**
 * \class ds::TcpClient
 */
TcpClient::TcpClient(const Poco::Net::SocketAddress& address)
{
	try {
		mLoop.mSocket.connect(address);
		mLoop.mSocket.setBlocking(false);
		mLoop.mSocket.setReceiveTimeout(Poco::Timespan(10, 0));
		
		mThread.start(mLoop);
	} catch (Poco::Exception&) {
//    std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
	} catch (std::exception&) {
//    std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
	}
}

TcpClient::~TcpClient()
{
	{
		Poco::Mutex::ScopedLock		l(mLoop.mMutex);
		if (!mThread.isRunning()) return;

		mLoop.mAbort = true;
		mLoop.mSocket.shutdown();
	}

	try {
		mThread.join();
	} catch (std::exception&) {
	}
}

void TcpClient::add(const std::function<void(const std::string&)>& f)
{
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

void TcpClient::update()
{
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
TcpClient::Loop::Loop()
	: mAbort(false)
	, mUpdates(0)
{
}

void TcpClient::Loop::run()
{
	static const int			BUF_SIZE = 4096;

	char						buf[BUF_SIZE];

	while (true) {
		int						length = 0;
		try
		{
			length = mSocket.receiveBytes(buf, BUF_SIZE);
			// I think a return of 0 here indicates that the connection is closed
		}
		catch (Poco::TimeoutException&)
		{
//      std::cout << "TcpWatcher timeout" << std::endl;
//      std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
		}
		catch (std::exception&)
		{
//      std::cout << "TcpWatcher exception" << std::endl;
//      std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
		}
		// Throttle this down, so I'm not slamming the mutex
		Poco::Thread::sleep(100);

		if ( length > 0 )
		{
			update(std::string(buf, length));
		}

		{
			Poco::Mutex::ScopedLock	l(mMutex);
			if (mAbort) break;
		}
	}
}

void TcpClient::Loop::update(const std::string& str)
{
	Poco::Mutex::ScopedLock	l(mMutex);
	try {
		mUpdates.push_back(str);
	} catch (std::exception&) {
	}
}

} // namespace net
} // namespace ds
