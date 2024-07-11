#pragma once

#include "file_list_save_service.h"
#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

/**
 * \class waffles::RecentFilesService
 * Load and save a list of files that have been opened recently
 */
class RecentFilesService {
  public:
	RecentFilesService(ds::ui::SpriteEngine&);

	void initialize();

  private:
	void addToRecentlyOpened(ds::model::ContentModelRef newMedia);

	ds::ui::SpriteEngine&					mEngine;
	ds::EventClient							mEventClient;
	ds::SerialRunnable<FileListSaveService> mSaveService;

	std::vector<ds::model::ContentModelRef> mRecentFiles;
};


} // namespace waffles
