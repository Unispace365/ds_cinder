#pragma once
#ifndef DS_NETWORK_NODEWATCHER_H_
#define DS_NETWORK_NODEWATCHER_H_

#include <functional>
#include <vector>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include "ds/app/auto_update.h"

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
		std::vector<std::string>	mData;

		bool						empty() const;
		void						clear();
		void						swap(Message&);
	};

public:
	/// Standard node location
	NodeWatcher(ds::ui::SpriteEngine&, const std::string& host = "localhost", const int port = 7777, const bool autoStart = true);
	~NodeWatcher();

	void							add(const std::function<void(const Message&)>&);
	void							startWatching();
	void							stopWatching();

protected:
	virtual void					update(const ds::UpdateParams &);

private:
	class Loop : public Poco::Runnable {
	public:
		Poco::Mutex					mMutex;
		bool						mAbort;
		Message						mMsg;

	public:
		Loop(ds::ui::SpriteEngine&, const std::string& host, const int port);

		virtual void				run();

	private:
		const std::string			mHost;
		const int					mPort;
		const long					mRefreshRateMs;	// in milliseconds
	};

	Poco::Thread					mThread;
	Loop							mLoop;
	std::vector<std::function<void(const Message&)>>
									mListener;
	Message							mMsg;
};

} // namespace ds

#endif // DS_NETWORK_NODEWATCHER_H_
