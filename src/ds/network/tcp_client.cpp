#include "stdafx.h"

#include "ds/network/tcp_client.h"

#include <iostream>
#include <thread>
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

namespace {

// Because the sockets are non-blocking, occasionally I will get write errors
// if I'm sending too much data. This loop doesn't guarantee that I won't lose
// data, but not sure what a realistic way to handle that would be. Need to balance
// remaining reponsive with making sure all data gets through.
size_t					send_loop(Poco::Net::StreamSocket& socket, std::string data) {
	const size_t		RETRY_MAX = 1000;
	for (size_t count=0; count < RETRY_MAX; ++count) {
		try {
			int			sent_size = socket.sendBytes(data.data(), static_cast<int>(data.size()));
			if (sent_size == static_cast<int>(data.size())) {
				return data.size();
			}
		} catch (Poco::IOException const &) {
//			std::cout << "send ioex=" << ex.what() << " t=" << ex.displayText() << std::endl;
		} catch (std::exception const &) {
//			std::cout << "send ex=" << ex.what() << std::endl;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	return 0;
}

}

void TcpClient::sendBytes(Poco::Net::StreamSocket &socket, const std::string &d) {
	try {
		if (d.empty()) return;

		// We can periodically get an IO error, which happens if we are sending
		// data too quickly, so retry.
		const size_t	buffer_size = static_cast<size_t>(socket.getSendBufferSize());
		size_t			pos = 0;
		while (pos < d.size()) {
			size_t		len = d.size()-pos;
			if (len > buffer_size) len = buffer_size;
			size_t		sent = send_loop(socket, d.substr(pos, len));
			if (sent != len) {
//				std::cout << "SEND FAILED=" << sent << " wanted=" << len << std::endl;
				return;
			}
			pos += sent;
		}
	} catch (std::exception const &) {
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
				mSocket.setNoDelay(true);
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
//				std::cout << "TcpClient timeout" << std::endl;
//				std::cout << "TcpClient ex=" << ex.what() << std::endl;
			} catch (Poco::Net::ConnectionAbortedException&) {
				needsConnect = true;
			} catch (Poco::Net::ConnectionResetException&) {
				needsConnect = true;
			} catch (Poco::Net::InvalidSocketException&) {
				needsConnect = true;
			} catch (std::exception&) {
//				std::cout << "TcpClient exception " << ex.what() << std::endl;
//				std::cout << "TcpClient ex=" << ex.what() << std::endl;
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
		// Get data to send
		std::vector<std::string>	data;
		{
			Poco::Mutex::ScopedLock	l(mMutex);
			data.swap(mSendData);
		}

		// Compile into a single chunk of data. Note that I've seen
		// messages not go through when we try to make a series
		// of sendBytes() calls, so this prevents that.
		std::stringstream				buf;
		for (const auto& d : data) {
			if (!d.empty()) buf << d << mTerminator;
		}

		TcpClient::sendBytes(socket, buf.str());
	} catch (std::exception const &) {
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
