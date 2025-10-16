#pragma once
#ifndef DS_NETWORK_NODEWATCHER_H_
#define DS_NETWORK_NODEWATCHER_H_

#include "ds/app/auto_update.h"
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <functional>
#include <vector>

namespace ds {

/**
 * \class NodeWatcher
 * \brief Feed clients information about changes in the node.
 */
class NodeWatcher : public ds::AutoUpdate {
  public:
	/// A generic class that stores info received from the node.
	class Message {
	  public:
		Message();

		/// A stack of all the messages received
		std::vector<std::string> mData;

		bool empty() const;
		void clear();
		void swap(Message&) noexcept;
	};

  public:
	/// Standard node location
	NodeWatcher(ds::ui::SpriteEngine&, const std::string& host = "localhost", uint16_t port = 7788,
				bool autoStart = true);
	~NodeWatcher() override;

	void add(const std::function<void(const Message&)>&);
	void startWatching();
	void stopWatching();

	std::string toString() const { return std::string("NodeWatcher@") + mLoop.toString(); }

  protected:
	void update(const ds::UpdateParams&) override;

  private:
	class Loop : public Poco::Runnable {
	  public:
		Poco::Mutex mMutex;
		bool		mAbort;
		Message		mMsg;

	  public:
		Loop(const ds::ui::SpriteEngine&, const std::string& host, uint16_t port);

		void run() override;

		const std::string& getHost() const { return mHost; }
		int				   getPort() const { return mPort; }

		std::string toString() const { return mHost + ":" + std::to_string(mPort); }

	  private:
		bool shouldAbort() {
			Poco::Mutex::ScopedLock l(mMutex);
			return mAbort;
		}

		bool initializeSocket(Poco::Net::DatagramSocket& socket) const {
			try {
				socket.bind(Poco::Net::SocketAddress(mHost, mPort), true, true);
				socket.setBlocking(false);
				socket.setReceiveTimeout(0);
				DS_LOG_INFO("DatagramSocket initialized for " << toString());
				return true;
			} catch (std::exception& e) {
				DS_LOG_WARNING("Failed to initialize DatagramSocket " << toString() << ": " << e.what());
				return false;
			}
		}

	  private:
		const std::string mHost;
		const uint16_t	  mPort;
		const long		  mRefreshRateMs; // in milliseconds
	};

	Poco::Thread									 mThread;
	Loop											 mLoop;
	std::vector<std::function<void(const Message&)>> mListener;
	Message											 mMsg;
};

} // namespace ds

#endif // DS_NETWORK_NODEWATCHER_H_
