#ifndef _SETTINGS_REWRITE_APP_H_
#define _SETTINGS_REWRITE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace downstream {
class AllData;
class StoryView;

class settings_rewrite_app : public ds::App {
public:
	settings_rewrite_app();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);

private:
	void				onAppEvent(const ds::Event&);

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	// App events can be handled here
	ds::EventClient		mEventClient;

	StoryView*			mStoryView;
};

} // !namespace downstream

#endif // !_SETTINGS_REWRITE_APP_H_
