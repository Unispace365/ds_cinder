#include "stdafx.h"

#include "ds/network/node_watcher.h"

#include <Poco/Net/DatagramSocket.h>
#include <ds/cfg/settings.h>
#include <ds/debug/debug_defines.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {

/**
 * \class ds::NodeWatcher
 */
NodeWatcher::NodeWatcher(ds::ui::SpriteEngine& se, const std::string& host, const int port)
		: ds::AutoUpdate(se)
		, mLoop(se, host, port) {
	mThread.start(mLoop);
}

NodeWatcher::~NodeWatcher() {
	{
		Poco::Mutex::ScopedLock		l(mLoop.mMutex);
		mLoop.mAbort = true;
		// I don't know how one would "wakeup" the socket.  If I knew that,
		// I could increase the timeout and make the whole thing more efficient.
//		mLoop.mCondition.signal();
	}
	try {
		mThread.join();
	} catch (std::exception&) {
	}
}

void NodeWatcher::add(const std::function<void(const Message&)>& f) {
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

void NodeWatcher::update(const ds::UpdateParams &) {
	mMsg.clear();
	{
		Poco::Mutex::ScopedLock	l(mLoop.mMutex);
		mMsg.swap(mLoop.mMsg);
	}
	if (mMsg.empty()) return;

	for (auto it=mListener.begin(), end=mListener.end(); it != end; ++it) {
		(*it)(mMsg);
	}
}

/**
 * \class ds::NodeWatcher::Loop
 */
static long get_refresh_rate(ds::ui::SpriteEngine& e) {
	// Default to one second
	const ds::cfg::Settings&		settings = e.getSettings("engine");
	float							rate = settings.getFloat("node:refresh_rate", 0, .1f);
	long							ans = static_cast<long>(rate * 1000.0f);
	if (ans < 10) return 10;
	else if (ans > 1000 * 10) return 1000 * 10;
	return ans;
}

NodeWatcher::Loop::Loop(ds::ui::SpriteEngine& e, const std::string& host, const int port)
		: mAbort(false)
		, mHost(host)
		, mPort(port)
		, mRefreshRateMs(get_refresh_rate(e)) {
}

// WARNING: this class can throw in constructor thanks to
// awesome architecture of Poco. Use with try-catch.
struct ScopedDatagramSocket
{
	ScopedDatagramSocket(const std::string& host, int port)
		: mCmsReceiver(Poco::Net::SocketAddress(host, port))
	{}

	~ScopedDatagramSocket() { try { mCmsReceiver.close(); } catch (...) {} }

	Poco::Net::DatagramSocket	mCmsReceiver;
};

void NodeWatcher::Loop::run() {
	static const int			BUF_SIZE = 512;
	char						buf[BUF_SIZE];

	try
	{
		ScopedDatagramSocket so{ mHost, mPort };
		
		so.mCmsReceiver.setBlocking(false);
		so.mCmsReceiver.setReceiveTimeout(0);

		while (true)
		{
			int						length = 0;

			try
			{
				length = so.mCmsReceiver.receiveBytes(buf, BUF_SIZE);
			}
			catch (const Poco::TimeoutException&)
			{
			}
			catch (const std::exception&)
			{
			}

			if (length > 0)
			{
				try
				{
					std::string		msg(buf, length);
					Poco::Mutex::ScopedLock	l(mMutex);
					mMsg.mData.push_back(msg);
				}
				catch (const std::exception&)
				{
				}
			}
			Poco::Thread::sleep(mRefreshRateMs);
			{
				Poco::Mutex::ScopedLock	l(mMutex);
				if (mAbort) break;
			}
		}
	}
	catch (...)
	{
		// WARNING: you must not use DS_LOG_XX here. We are in a thread and logger class
		// is not thread-safe.
		std::cerr << "Unable to construct the DatagramSocket to DS Node." << std::endl;
	}
}

/**
 * \class ds::NodeWatcher::Message
 */
NodeWatcher::Message::Message()
{
}

bool NodeWatcher::Message::empty() const
{
	return mData.empty();
}

void NodeWatcher::Message::clear()
{
	mData.clear();
}

void NodeWatcher::Message::swap(Message& o)
{
	mData.swap(o.mData);
}

} // namespace ds
