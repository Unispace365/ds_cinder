#include "stdafx.h"

#include "directory_watcher.h"

#include <iostream>
#include <Windows.h>

using namespace std;
using namespace ds;

static VOID CALLBACK empty_func(ULONG_PTR dwParam)	{ }

namespace {
// OK It's horrible placing this as a global, but there can only ever be a single
// watcher thread going, and I don't want to clutter the API with platform references.
Poco::Mutex			WAKEUP_LOCK;
static HANDLE		WAKEUP = INVALID_HANDLE_VALUE;

void				setWakeup(HANDLE h) {
	Poco::Mutex::ScopedLock		l(WAKEUP_LOCK);
	WAKEUP = h;
}

void				signalWakeup() {
	Poco::Mutex::ScopedLock		l(WAKEUP_LOCK);
	if (WAKEUP != INVALID_HANDLE_VALUE) SetEvent(WAKEUP);
}

}

/**
 * \class ds::DirectoryWatcher
 */
void DirectoryWatcher::wakeup()
{
	signalWakeup();
}

/**
 * \class ds::DirectoryWatcherOp
 */
void DirectoryWatcher::Waiter::run()
{
//	cout << "START DIRECTORY WATCHER" << endl;
	DWORD				ans;
	HANDLE*				handle;
	DWORD				notify = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME
								 | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE
								 | FILE_NOTIFY_CHANGE_LAST_WRITE;
	size_t				k, count = mPath.size();
	if (count < 1) return;
	// Insert my own handle I can wakeup
	++count;
	if ((handle = new HANDLE[count]) == 0) return;

	handle[0] = CreateEventW(0, FALSE, FALSE, 0);
	setWakeup(handle[0]);

	for (k = 1; k < count; k++) {
		handle[k] = FindFirstChangeNotificationA(mPath[k-1].c_str(), true, notify);
	}
	if (handle[0] == INVALID_HANDLE_VALUE) goto cleanup;
	for (k = 1; k < count; k++) {
		if (handle[k] == INVALID_HANDLE_VALUE) {
			cout << "ERROR DirectoryWatcherOp invalid handle on " << mPath[k-1] << endl;
			goto cleanup;
		}
	}
//	cout << "LOOP DIRECTORY WATCHER" << endl;

	while (TRUE) { 
		// Wait for notification.
		ans = WaitForMultipleObjectsEx(count, handle, FALSE, INFINITE, TRUE);

		if (isStopped()) goto cleanup;
		if (ans < WAIT_OBJECT_0 || ans >= WAIT_OBJECT_0 + count) goto cleanup;
		ans -= WAIT_OBJECT_0;

		if (ans >= 1 && ans < count) {
//			cout << "CHANGED=" << mPath[ans-1] << endl;
			if (!onChanged(mPath[ans-1])) goto cleanup;
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
		if (handle[k] != INVALID_HANDLE_VALUE)
			FindCloseChangeNotification(handle[k]);
	}
	delete[] handle;
}
