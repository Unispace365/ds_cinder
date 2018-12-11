#include "stdafx.h"

#include "directory_watcher.h"

#include <algorithm>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {

DirectoryWatcher::DirectoryWatcher(ds::ui::SpriteEngine& se)
	: ds::AutoUpdate(se)
	, mStop(0)
	, mWaiter(mStop, se.getNotifier()) {
}

DirectoryWatcher::~DirectoryWatcher() {
	stop();
}

void DirectoryWatcher::addPath(const std::string& path) {
	try {
		if (!path.empty()) mWaiter.mPaths.push_back(path);
	} catch (std::exception& ex) {
		DS_LOG_WARNING("DirectoryWatcher::addPath() exception: " << ex.what());
	}
}

void DirectoryWatcher::clearPaths() {
	try {
		mWaiter.mPaths.clear();
	} catch(std::exception& ex) {
		DS_LOG_WARNING("DirectoryWatcher::clearPaths() exception: " << ex.what());
	}
}

void DirectoryWatcher::start() {
	if (!mThread.isRunning()) {
		mStop = 0;
		mThread.start(mWaiter);
	}
}

void DirectoryWatcher::stop() {
	mStop++;
	wakeup();
	try {
		mThread.join();
	} catch (std::exception&) {
	}
}

void DirectoryWatcher::update(const ds::UpdateParams&) {
	mWaiter.update();
}

/**
 * \class Waiter
 * \brief Handle waiting on directory changes and sending notices.
 */
DirectoryWatcher::Waiter::Waiter(	const Poco::AtomicCounter& stop,
									ds::EventNotifier& n)
		: mStop(stop)
		, mNotifier(n) {
}

void DirectoryWatcher::Waiter::update() {
	mLocalPaths.clear();
	{
		Poco::Mutex::ScopedLock		lock(mLock);
		mLocalPaths.swap(mChangedPaths);
	}
	if (mLocalPaths.empty()) return;
	for (auto it=mLocalPaths.begin(), end=mLocalPaths.end(); it!=end; ++it) {
		mNotifier.notify(Changed(*it));
	}
}

bool DirectoryWatcher::Waiter::isStopped() {
	return mStop.value() > 0;
}

bool DirectoryWatcher::Waiter::onChanged(const std::string& path) {
	Poco::Mutex::ScopedLock		lock(mLock);
	if (std::find(mChangedPaths.begin(), mChangedPaths.end(), path) == mChangedPaths.end()) {
		mChangedPaths.push_back(path);
	}
	return true;
}

} // namespace ds
