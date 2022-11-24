#include "stdafx.h"

#include "directory_watcher.h"

#include <Windows.h>
#include <iostream>

#include <ds/debug/logger.h>

// using namespace std;
using namespace ds;

static VOID CALLBACK empty_func(ULONG_PTR dwParam) {}

namespace {
// OK It's horrible placing this as a global, but there can only ever be a single
// watcher thread going, and I don't want to clutter the API with platform references.
Poco::Mutex	  WAKEUP_LOCK;
static HANDLE WAKEUP = INVALID_HANDLE_VALUE;

void setWakeup(HANDLE h) {
	Poco::Mutex::ScopedLock l(WAKEUP_LOCK);
	WAKEUP = h;
}

void signalWakeup() {
	Poco::Mutex::ScopedLock l(WAKEUP_LOCK);
	if (WAKEUP != INVALID_HANDLE_VALUE) SetEvent(WAKEUP);
}

} // namespace

/**
 * \class DirectoryWatcher
 */
void DirectoryWatcher::wakeup() {
	signalWakeup();
}

/**
 * \class DirectoryWatcherOp
 */
void DirectoryWatcher::Waiter::run() {
	//	cout << "START DIRECTORY WATCHER" << endl;
	DWORD	ans;
	HANDLE* handle;
	DWORD	notify = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
				   FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
	size_t k, count = mPaths.size();
	if (count < 1) return;
	// Insert my own handle I can wakeup
	++count;
	if ((handle = new HANDLE[count]) == 0) return;

	handle[0] = CreateEventW(0, FALSE, FALSE, 0);
	setWakeup(handle[0]);

	for (k = 1; k < count; k++) {
		handle[k] = FindFirstChangeNotificationA(mPaths[k - 1].c_str(), true, notify);
	}
	if (handle[0] == INVALID_HANDLE_VALUE) goto cleanup;
	for (k = 1; k < count; k++) {
		if (handle[k] == INVALID_HANDLE_VALUE) {
			std::cout << "ERROR DirectoryWatcherOp invalid handle on " << mPaths[k - 1] << std::endl;
			goto cleanup;
		}
	}
	//	cout << "LOOP DIRECTORY WATCHER" << endl;

	while (TRUE) {
		// Wait for notification.
		ans = WaitForMultipleObjectsEx((DWORD)count, handle, FALSE, INFINITE, TRUE);

		if (isStopped()) goto cleanup;
		if (ans < WAIT_OBJECT_0 || ans >= WAIT_OBJECT_0 + count) goto cleanup;
		ans -= WAIT_OBJECT_0;

		if (ans >= 1 && ans < count) {
			DS_LOG_VERBOSE(3, "DirectoryWatcherWin32:: CHANGED=" << mPaths[ans - 1]);
			if (!onChanged(mPaths[ans - 1])) goto cleanup;
		}

		if (FindNextChangeNotification(handle[ans]) == FALSE) goto cleanup;
	}

cleanup:
	//	cout << "END DIRECTORY WATCHER" << endl;
	setWakeup(INVALID_HANDLE_VALUE);
	if (handle[0] != INVALID_HANDLE_VALUE) {
		CloseHandle(handle[0]);
	}
	for (k = 1; k < count; k++) {
		if (handle[k] != INVALID_HANDLE_VALUE) FindCloseChangeNotification(handle[k]);
	}
	delete[] handle;
}
