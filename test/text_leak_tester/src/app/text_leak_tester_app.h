#ifndef _TEXT_LEAK_TESTER_APP_H_
#define _TEXT_LEAK_TESTER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include "app/globals.h"
#include "query/query_handler.h"

namespace downstream {
class AllData;

class text_leak_tester_app : public ds::App {
public:
	text_leak_tester_app();
	~text_leak_tester_app();

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

} // !namespace downstream

#endif // !_TEXT_LEAK_TESTER_APP_H_
