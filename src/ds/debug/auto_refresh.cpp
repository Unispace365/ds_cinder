#include "stdafx.h"

#include "ds/debug/auto_refresh.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {


AutoRefresh::AutoRefresh(ui::SpriteEngine& eng)
  : mEngine(eng)
  , mDirectoryWatcher(eng)
  , mEventClient(eng) {

	mEventClient.listenToEvents<DirectoryWatcher::Changed>([this](const DirectoryWatcher::Changed& e) {
		auto doRefresh = mEngine.getEngineSettings().getBool("auto_refresh_app");
		if (doRefresh) {
			for (const auto& it : mWatchPaths) {
				if (e.mPath == Environment::expand(it)) {
					mEngine.restartAfterNextUpdate();
				}
			}
		}
	});
}

void AutoRefresh::initialize() {

	mDirectoryWatcher.stop();
	mDirectoryWatcher.clearPaths();

	auto doRefresh = mEngine.getEngineSettings().getBool("auto_refresh_app");
	auto allPaths  = mEngine.getEngineSettings().getString("auto_refresh_directories");

	DS_LOG_VERBOSE(1, "AutoRefresh: auto_refresh_app is " << doRefresh << " with paths " << allPaths);

	if (!allPaths.empty()) {
		mWatchPaths = split(allPaths, ";", true);
		if (!mWatchPaths.empty()) {
			for (const auto& it : mWatchPaths) {
				mDirectoryWatcher.addPath(Environment::expand(it));
			}

			mDirectoryWatcher.start();
		} else {
			DS_LOG_WARNING("AutoRefresh: watch paths specified, but could not be parsed");
		}
	}
}

} // namespace ds