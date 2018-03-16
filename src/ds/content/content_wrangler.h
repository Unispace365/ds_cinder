#pragma once
#ifndef DS_CONTENT_CONTENT_WRANGLER
#define DS_CONTENT_CONTENT_WRANGLER

#include <ds/thread/serial_runnable.h>
#include <ds/network/helper/delayed_node_watcher.h>
#include <ds/app/event_client.h>

#include "content_model.h"
#include "content_query.h"

namespace ds {
namespace ui {
class SpriteEngine;
}

/**
* \class downstream::ContentWrangler
* \brief Listen to dsnode and app events to run queries and notify about the results
*		 Does nothing if no resource location has been specified
*/
class ContentWrangler {
public:
	ContentWrangler(ds::ui::SpriteEngine&);

	// TODO: handle errors from the content query (don't replace mData or send out update events)
	// TODO: detect when a query is already running and don't try to start a new one (could have a situation where the previous query never finishes)

	/// A map of all the resources from the resources table
	std::unordered_map<int, ds::Resource>	mAllResources; 

	/// Starts node watcher and sets xml / db locations
	void									initialize();

	/// Asynchronously runs query and notifies the ContentUpdatedEvent when complete
	void									runQuery();

private:
	ds::ui::SpriteEngine&					mEngine;
	ds::SerialRunnable<ContentQuery>		mContentQuery;

	ds::DelayedNodeWatcher					mNodeWatcher;
	ds::EventClient							mEventClient;

	std::string								mModelModelLocation;
};

} // !namespace ds

#endif 
