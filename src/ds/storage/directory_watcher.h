#pragma once
#ifndef DS_STORAGE_DIRECTORYWATCHER_H_
#define DS_STORAGE_DIRECTORYWATCHER_H_

#include <functional>
#include <string>
#include <vector>
#include <Poco/AtomicCounter.h>
#include <Poco/Mutex.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <ds/app/auto_update.h>
#include <ds/app/event.h>
#include <ds/app/event_notifier.h>

namespace ds {

/**
 * \class ds::DirectoryWatcher
 */
class DirectoryWatcher : public ds::AutoUpdate {
// Change event
public:
	class Changed : public ds::RegisteredEvent<Changed> {
	public:
		Changed(const std::string& path) : mPath(path) {}
		const std::string& mPath;
	};

public:
	DirectoryWatcher(ds::ui::SpriteEngine&);
	~DirectoryWatcher();

	// NOTE:  addPath is initialization only.  As soon as you start, don't use it.
	// Why?  I guess I'm cheap that way.  It's not currently thread safe.
	void						addPath(const std::string& path);

	// Must be called while the directory watcher is stopped
	void						clearPaths();

	void						start();
	void						stop();

protected:
	virtual void				update(const ds::UpdateParams&);

private:

class Waiter : public Poco::Runnable {
public:
	// Directories I'm watching
	std::vector<std::string>	mPaths;

public:
	Waiter(const Poco::AtomicCounter&, ds::EventNotifier&);

	// The platform implementation is responsible for suppling a run().
	virtual void				run();
	void						update();

protected:

	bool						isStopped();
	bool						onChanged(const std::string& path);

private:
	const Poco::AtomicCounter&	mStop;

	std::vector<std::string>	mLocalPaths;
	// Shared between worker and main threads.
	Poco::Mutex					mLock;
	std::vector<std::string>	mChangedPaths;
	// Only call from the main thread
	ds::EventNotifier&			mNotifier;
};

private:
	Poco::AtomicCounter			mStop;
	Poco::Thread				mThread;
	Waiter						mWaiter;

	// The platform implementation is responsible for waking up the thread.
	void						wakeup();
};

} // namespace ds

#endif
