#include "query_handler.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <cinder/Json.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include "app/app_defs.h"
#include "app/globals.h"

namespace mv {

/**
 * \class mv::QueryHandler
 */
QueryHandler::QueryHandler(ds::ui::SpriteEngine& se, AllData &ad)
		: mEventClient(se.getNotifier(), [this](const ds::Event* e){if (e) onAppEvent(*e); })
		, mAllData(ad)
		, mMediaQuery(se, [](){return new MediaQuery(); })
{

	// Initialize data
	mMediaQuery.setReplyHandler([this](MediaQuery& q){this->onMediaQuery(q); });
	mMediaQuery.start(nullptr, true);
}

void QueryHandler::onAppEvent(const ds::Event& _e) {
}

void QueryHandler::onMediaQuery(MediaQuery& q) {
	mAllData = q.mOutput;
}


} // !namespace mv


