#pragma once
#ifndef DS_CONTENT_CONTENT_WRANGLER
#define DS_CONTENT_CONTENT_WRANGLER

#include <ds/app/event_client.h>
#include <ds/content/content_query.h>
#include <ds/network/helper/delayed_node_watcher.h>
#include <ds/thread/parallel_runnable.h>

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
	ContentWrangler(ui::SpriteEngine&);

	/// TODO: handle errors from the content query (don't replace mData or send out update events)

	/// A map of all the resources from the resources table
	std::unordered_map<int, Resource> mAllResources;

	/// Starts node watcher and sets xml / db locations
	void initialize();

	/// Reply handler for individual queries
	[[deprecated("Use receiveQuery() instead")]] void recieveQuery(ContentQuery& q) { receiveQuery(q); }
	/// Reply handler for individual queries
	void receiveQuery(ContentQuery& q) const;

	/// Asynchronously runs query and notifies the ContentUpdatedEvent when complete
	void runQuery();


  private:
	ui::SpriteEngine&			   mEngine;
	ParallelRunnable<ContentQuery> mContentQuery;

	DelayedNodeWatcher mNodeWatcher;
	EventClient		   mEventClient;

	std::string mModelModelLocation;
};

} // namespace ds

#endif
