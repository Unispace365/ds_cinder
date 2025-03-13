#pragma once
#ifndef DS_STORAGE_DIRECTORYWATCHER_H_
#define DS_STORAGE_DIRECTORYWATCHER_H_

#include <functional>
#include <string>
#include <vector>

#include "ds/app/auto_update.h"
#include "ds/app/event.h"
#include "ds/app/event_notifier.h"

#include <Poco/AtomicCounter.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>

namespace ds {

/**
 * \class DirectoryWatcher
 */
class DirectoryWatcher : public AutoUpdate {
	// Change event
  public:
	class Changed : public RegisteredEvent<Changed> {
	  public:
		Changed(const std::string& path)
		  : mPath(path) {}
		const std::string& mPath;
	};

  public:
	DirectoryWatcher(ui::SpriteEngine&);
	~DirectoryWatcher() override;

	/// NOTE:  addPath is initialization only.  As soon as you start, don't use it.
	/// Why?  I guess I'm cheap that way.  It's not currently thread safe.
	void addPath(const std::string& path);

	/// Must be called while the directory watcher is stopped
	void clearPaths();

	void start();
	void stop();

  protected:
	void update(const UpdateParams&) override;

  private:
	class Waiter : public Poco::Runnable {
	  public:
		/// Directories I'm watching
		std::vector<std::string> mPaths;

	  public:
		Waiter(const Poco::AtomicCounter&, EventNotifier&);

		/// The platform implementation is responsible for supplying a run().
		void run() override;
		void update();

	  protected:
		bool isStopped() const;
		bool onChanged(const std::string& path);

	  private:
		const Poco::AtomicCounter& mStop;

		std::vector<std::string> mLocalPaths;
		/// Shared between worker and main threads.
		Poco::Mutex				 mLock;
		std::vector<std::string> mChangedPaths;
		/// Only call from the main thread
		EventNotifier& mNotifier;
	};

  private:
	Poco::AtomicCounter mStop;
	Poco::Thread		mThread;
	Waiter				mWaiter;

	/// The platform implementation is responsible for waking up the thread.
	static void wakeup();
};

} // namespace ds

#endif
