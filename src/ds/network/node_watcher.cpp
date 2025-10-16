#include "stdafx.h"

#include "ds/network/node_watcher.h"

#include <Poco/Net/DatagramSocket.h>
#include <ds/cfg/settings.h>
#include <ds/debug/debug_defines.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {

/**
 * \class NodeWatcher
 */
NodeWatcher::NodeWatcher(ds::ui::SpriteEngine& se, const std::string& host, uint16_t port, bool autoStart)
  : ds::AutoUpdate(se)
  , mLoop(se, host, port) {
	if (autoStart) {
		startWatching();
	}
}

NodeWatcher::~NodeWatcher() {
	{
		Poco::Mutex::ScopedLock l(mLoop.mMutex);
		mLoop.mAbort = true;
		mThread.wakeUp();
	}

	try {
		mThread.join();
	} catch (std::exception&) {}
}

void NodeWatcher::add(const std::function<void(const Message&)>& f) {
	if (!f) return;

	try {
		mListener.emplace_back(f);
	} catch (std::exception& ex) {
		DS_LOG_WARNING("NodeWatcher::add() Couldn't add a listener function with exception " << ex.what());
	}
}

void NodeWatcher::startWatching() {
	if (mThread.isRunning()) return;

	try {
		DS_LOG_VERBOSE(2, toString() + ": Start watching");
		mThread.start(mLoop);
	} catch (std::exception& ex) {
		DS_LOG_WARNING("NodeWatcher::startWatching() Couldn't with exception " << ex.what());
	}
}

void NodeWatcher::stopWatching() {
	if (!mThread.isRunning()) return;
	try {
		Poco::Mutex::ScopedLock l(mLoop.mMutex);
		DS_LOG_VERBOSE(2, toString() + ": Stop watching");
		mLoop.mAbort = true;
		mThread.wakeUp();

	} catch (std::exception& ex) {
		DS_LOG_WARNING("NodeWatcher::stopWatching() Couldn't with exception " << ex.what());
	}
}

void NodeWatcher::update(const ds::UpdateParams&) {
	mMsg.clear();
	{
		Poco::Mutex::ScopedLock l(mLoop.mMutex);
		mMsg.swap(mLoop.mMsg);
	}
	if (mMsg.empty()) return;

	if (ds::Logger::hasVerboseLevel(1)) {
		for (const auto& it : mMsg.mData) {
			DS_LOG_VERBOSE(1, toString() + ": got message: " << it);
		}
	}

	for (const auto& it : mListener) {
		it(mMsg);
	}
}

namespace {

	static long get_refresh_rate(const ds::ui::SpriteEngine& e) {
		// Default to one second
		ds::cfg::Settings& settings = e.getEngineSettings();
		float			   rate		= settings.getFloat("node:refresh_rate", 0, .1f);
		long			   ans		= static_cast<long>(rate * 1000.0f);
		if (ans < 10)
			return 10;
		else if (ans > 1000 * 10)
			return 1000 * 10;
		return ans;
	}

} // namespace

NodeWatcher::Loop::Loop(const ds::ui::SpriteEngine& e, const std::string& host, uint16_t port)
  : mAbort(false)
  , mHost(host)
  , mPort(port)
  , mRefreshRateMs(get_refresh_rate(e)) {}

void NodeWatcher::Loop::run() {
	static constexpr int BUF_SIZE = 512;
	char				 buf[BUF_SIZE];

	Poco::Net::DatagramSocket theSocket;

	try {


		// theSocket.setReuseAddress(true);
		// theSocket.setReusePort(true);
		theSocket.bind(Poco::Net::SocketAddress(mHost, mPort), true, true);
		theSocket.setBlocking(false);
		theSocket.setReceiveTimeout(0);

		while (true) {
			int length = 0;

			try {
				length = theSocket.receiveBytes(buf, BUF_SIZE);
			} catch (const Poco::TimeoutException&) {
			} catch (const std::exception&) {}

			if (length > 0) {
				try {
					std::string				msg(buf, length);
					Poco::Mutex::ScopedLock l(mMutex);
					mMsg.mData.emplace_back(msg);
				} catch (const std::exception&) {}
			}

			Poco::Thread::trySleep(mRefreshRateMs);

			{
				Poco::Mutex::ScopedLock l(mMutex);
				if (mAbort) break;
			}
		}
	} catch (std::exception& e) {
		DS_LOG_WARNING("Unable to construct the DatagramSocket to DS Node. " << e.what());
	}

	try {
		// theSocket.close();
	} catch (std::exception& e) {
		DS_LOG_WARNING("Exception closing node watcher datagram socket: " << e.what());
	}
}

/**
 * \class Message
 */
NodeWatcher::Message::Message() = default;

bool NodeWatcher::Message::empty() const {
	return mData.empty();
}

void NodeWatcher::Message::clear() {
	mData.clear();
}

void NodeWatcher::Message::swap(Message& o) noexcept {
	mData.swap(o.mData);
}

} // namespace ds
