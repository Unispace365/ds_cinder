#include "ds/network/node_watcher.h"

#include <Poco/Net/DatagramSocket.h>
#include "ds/debug/debug_defines.h"

namespace ds {

/**
 * \class ds::NodeWatcher
 */
NodeWatcher::NodeWatcher(ds::ui::SpriteEngine& se, const std::string& host, const int port)
//	: ds::AutoUpdate(se)
  : mLoop(host, port)
{
	mThread.start(mLoop);
}

NodeWatcher::~NodeWatcher()
{
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

void NodeWatcher::add(const std::function<void(const Message&)>& f)
{
	if (!f) return;

	try {
		mListener.push_back(f);
	} catch (std::exception&) {
	}
}

void NodeWatcher::update(const Poco::Timestamp::TimeVal&)
{
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
NodeWatcher::Loop::Loop(const std::string& host, const int port)
	: mAbort(false)
	, mHost(host)
	, mPort(port)
{
}

void NodeWatcher::Loop::run()
{
	static const int			BUF_SIZE = 512;

	Poco::Net::DatagramSocket	cmsReceiver;
	char						buf[BUF_SIZE];
	Poco::Net::SocketAddress	address;

	cmsReceiver.bind( Poco::Net::SocketAddress(mHost, mPort) );
	cmsReceiver.setBlocking(false);
//	cmsReceiver.setReceiveTimeout( Poco::Timespan(1000000*500) );
	cmsReceiver.setReceiveTimeout(Poco::Timespan(0));

	while (true) {
		int						length = 0;
		try
		{
			length = cmsReceiver.receiveFrom(buf, BUF_SIZE, address);
		}
		catch (Poco::TimeoutException&)
		{
		}
		catch (std::exception&)
		{
		}

		if ( length > 0 )
		{
			try {
				std::string		msg(buf, length);
				DS_DBG_CODE(std::cout << "DsNode watcher receive: " << msg << std::endl);
				Poco::Mutex::ScopedLock	l(mMutex);
				mMsg.mData.push_back(msg);
			} catch (std::exception&) {
			}
		}
		// Sleep for a second, I guess.
		Poco::Thread::sleep(1000*1);
		{
			Poco::Mutex::ScopedLock	l(mMutex);
			if (mAbort) break;
		}
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
