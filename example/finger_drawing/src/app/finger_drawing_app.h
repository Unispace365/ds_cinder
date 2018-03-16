#ifndef _FINGER_DRAWING_APP_H_
#define _FINGER_DRAWING_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace example {
class AllData;

class finger_drawing : public ds::App {
public:
	finger_drawing();

	void				setupServer();

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

} // !namespace example

#endif // !_FINGER_DRAWING_APP_H_