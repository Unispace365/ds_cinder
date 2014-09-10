#include "ds/network/tcp_client.h"

#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <Poco/Net/NetException.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>

namespace ds {
namespace net {

/**
 * \class ds::TcpClient
 */
TcpClient::TcpClient(	ds::ui::SpriteEngine& e, const Poco::Net::SocketAddress& address,
						const Options& opt, const std::string &terminator)
		: ds::AutoUpdate(e)
		, mLoop(address, opt, terminator) {
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
	} catch (std::exception const&) {
	}
}

void TcpClient::send(const std::string& data) {
	try {
		Poco::Mutex::ScopedLock	l(mLoop.mMutex);
		mLoop.mSendData.push_back(data);
	} catch (std::exception const&) {
	}
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
TcpClient::Loop::Loop(const Poco::Net::SocketAddress& a, const Options& opt, const std::string &terminator)
		: mAbort(false)
		, mUpdates(0)
		, mAddress(a)
		, mOptions(opt)
		, mTerminator(terminator) {
}

void TcpClient::Loop::run() {
	static const int			BUF_SIZE = 4096;

	char						buf[BUF_SIZE];
	bool						needsConnect = true;
	const Poco::Timespan		connect_timeout(1 * 100000);
	const double				opt_poll = (mOptions.mPollRate > 0.0 ? mOptions.mPollRate : 1.0);
	const long					poll_rate(static_cast<long>(opt_poll * 1000.0));
	while (true) {
		// Keep retrying the connection whenever it fails.
		if (needsConnect) {
			needsConnect = false;
			try {
				mSocket = Poco::Net::StreamSocket();
				mSocket.connect(mAddress, connect_timeout);
				mSocket.setBlocking(false);
				if (mOptions.mReceiveBufferSize > 0) mSocket.setReceiveBufferSize(mOptions.mReceiveBufferSize);
				if (mOptions.mSendBufferSize > 0) mSocket.setSendBufferSize(mOptions.mSendBufferSize);
			} catch (std::exception&) {
				needsConnect = true;
//				std::cout << "CONNECT ERROR=" << ex.what() << std::endl;
			}
		}

		int						length = 0;
		if (!needsConnect) {
			try {
				sendTo(mSocket);
				length = mSocket.receiveBytes(buf, BUF_SIZE);
			} catch (Poco::TimeoutException&) {
//				std::cout << "TcpWatcher timeout" << std::endl;
//				std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
			} catch (Poco::Net::ConnectionAbortedException&) {
				needsConnect = true;
			} catch (Poco::Net::ConnectionResetException&) {
				needsConnect = true;
			} catch (Poco::Net::InvalidSocketException&) {
				needsConnect = true;
			} catch (std::exception&) {
//				std::cout << "TcpWatcher exception " << ex.what() << std::endl;
//				std::cout << "TcpWatcher ex=" << ex.what() << std::endl;
			}
		}
		// Throttle this down, so I'm not slamming the mutex
		Poco::Thread::sleep(poll_rate);

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

void TcpClient::Loop::sendTo(Poco::Net::StreamSocket& socket) {
	try {
		std::vector<std::string>	data;
		{
			Poco::Mutex::ScopedLock	l(mMutex);
			data.swap(mSendData);
		}
		for (auto dit = data.begin(), dend=data.end(); dit!=dend; ++dit) {
			const std::string&		d(*dit);
			socket.sendBytes(d.data(), d.size());
		}
	} catch (std::exception const&) {
	}
}

void TcpClient::Loop::update(const std::string& str) {
	if (mTerminator.empty()) {
		Poco::Mutex::ScopedLock	l(mMutex);
		try {
			mUpdates.push_back(str);
		} catch (std::exception&) {
		}
	} else {
		// If I've got a terminator, then split the string based on the
		// terminator, and hold onto the last token if it doesn't have the terminator.
		mWaiting += str;
		std::vector<std::string> all;
		boost::split(all, mWaiting, boost::is_any_of(mTerminator));
		mWaiting.clear();
		// The last element will be an empty string if this update() str ended
		// with the terminator; if it's not, then it's a partial, so track that.
		if (!all.empty() && !all.back().empty()) {
			mWaiting = all.back();
			all.pop_back();
		}
		Poco::Mutex::ScopedLock	l(mMutex);
		try {
			for (auto it=all.begin(), end=all.end(); it!=end; ++it) {
				if (it->empty()) continue;
				mUpdates.push_back(*it);
			}
		} catch (std::exception&) {
		}
	}
}

/**
 * \class ds::NodeWatcher::Options
 */
TcpClient::Options::Options(const double poll_rate, const int receive_buffer_size, const int send_buffer_size)
		: mPollRate(poll_rate)
		, mReceiveBufferSize(receive_buffer_size)
		, mSendBufferSize(send_buffer_size) {
}

} // namespace net
} // namespace ds
