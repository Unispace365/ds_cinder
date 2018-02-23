#ifndef _PHYSICS_EXAMPLE_APP_H_
#define _PHYSICS_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace physics {
class AllData;

class physics_example_app : public ds::App {
public:
	physics_example_app();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

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

} // !namespace physics

#endif // !_PHYSICS_EXAMPLE_APP_H_
