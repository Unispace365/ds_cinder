#pragma once
#ifndef DS_CONTENT_CONTENT_WRANGLER
#define DS_CONTENT_CONTENT_WRANGLER

#include <ds/app/event_client.h>
#include <ds/network/helper/delayed_node_watcher.h>
#include <ds/thread/parallel_runnable.h>

#include "content_model.h"
#include "content_query.h"

namespace ds {
namespace ui {
class SpriteEngine;
}

/**
 * \class ContentWrangler
 * \brief Listen to dsnode and app events to run queries and notify about the results
 *		 Does nothing if no resource location has been specified
 */
class ContentWrangler {
  public:
	ContentWrangler(ds::ui::SpriteEngine&);

	/// TODO: handle errors from the content query (don't replace mData or send out update events)

	/// A map of all the resources from the resources table
	std::unordered_map<int, ds::Resource> mAllResources;

	/// Starts node watcher and sets xml / db locations
	void initialize();

	/// Reply handler for individual queries
	void recieveQuery(ContentQuery& q);

	/// Asynchronously runs query and notifies the ContentUpdatedEvent when complete
	void runQuery();

  private:
	ds::ui::SpriteEngine&              mEngine;
	ds::ParallelRunnable<ContentQuery> mContentQuery;

	ds::DelayedNodeWatcher mNodeWatcher;
	ds::EventClient        mEventClient;

	std::string mModelModelLocation;
};

}  // namespace ds

#endif
