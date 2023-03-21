#include "query_handler.h"

#include "app/app_defs.h"
#include "app/globals.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cinder/Json.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>

namespace panoramic {

/**
 * \class panoramic::QueryHandler
 */
QueryHandler::QueryHandler(ds::ui::SpriteEngine& se, AllData& ad)
  : mEventClient(se.getNotifier(),
				 [this](const ds::Event* e) {
					 if (e) onAppEvent(*e);
				 })
  , mAllData(ad)
  , mDirectoryQuery(se, []() { return new DirectoryLoader(); }) {

	// Initialize data
	mDirectoryQuery.setReplyHandler([this](DirectoryLoader& q) { this->onDirectoryQuery(q); });
}

void QueryHandler::runInitialQueries() {
	mDirectoryQuery.start(nullptr, true);
}

void QueryHandler::onAppEvent(const ds::Event& _e) {}

void QueryHandler::onDirectoryQuery(DirectoryLoader& q) {
	mAllData.mAllVideos = q.mOutput.mAllVideos;
}


} // namespace panoramic
