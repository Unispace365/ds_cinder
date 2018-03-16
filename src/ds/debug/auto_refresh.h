#pragma once
#ifndef DS_DEBUG_AUTO_REFRESH
#define DS_DEBUG_AUTO_REFRESH

#include <ds/storage/directory_watcher.h>
#include <ds/app/event_client.h>

namespace ds {
namespace ui {
class SpriteEngine;
}

/**
* \class ds::AutoRefresh
* \brief Listens to directory changes and soft restarts the app.
*/
class AutoRefresh {
public:
	AutoRefresh(ds::ui::SpriteEngine&);

	void							initialize();

private:
	ds::ui::SpriteEngine&			mEngine;
	ds::DirectoryWatcher			mDirectoryWatcher;
	ds::EventClient					mEventClient;
	std::vector<std::string>		mWatchPaths;
};

} // !namespace downstream

#endif 
