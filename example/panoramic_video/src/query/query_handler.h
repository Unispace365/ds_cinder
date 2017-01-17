#ifndef _PANORAMICVIDEO_APP_QUERY_QUERYHANDLER_H_
#define _PANORAMICVIDEO_APP_QUERY_QUERYHANDLER_H_

#include <ds/app/event_client.h>
#include <ds/data/resource_list.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>
// NOTE: Placing this include at the top gets a conflict
// with cinder. Need to look into that.
#include <ds/network/node_watcher.h>
#include "query/directory_loader.h"
#include "model/all_data.h"

namespace panoramic {

/**
 * \class panoramic::QueryHandler
 * \brief Handle app events that deal with querying for data.
 */
class QueryHandler {
public:
	QueryHandler(ds::ui::SpriteEngine&, AllData&);

	void								runInitialQueries();

private:

	void								onAppEvent(const ds::Event&);
	void								onDirectoryQuery(DirectoryLoader&);

	ds::EventClient						mEventClient;

	AllData&							mAllData;

	ds::SerialRunnable<DirectoryLoader>	mDirectoryQuery;

	// CACHING
	ds::ResourceList					mResources;
};

} // !namespace panoramic

#endif // !_PANORAMICVIDEO_APP_QUERY_QUERYHANDLER_H_

