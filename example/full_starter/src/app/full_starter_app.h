#ifndef _FULLSTARTER_APP_H_
#define _FULLSTARTER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace fullstarter {
class AllData;

class FullStarterApp : public ds::App {
public:
	FullStarterApp();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	void				onAppEvent(const ds::Event&);

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_H_