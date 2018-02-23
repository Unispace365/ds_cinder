#ifndef _PDF_LEAK_TESTER_APP_H_
#define _PDF_LEAK_TESTER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

#include "app/globals.h"

namespace downstream {
class AllData;

class pdf_leak_tester_app : public ds::App {
public:
	pdf_leak_tester_app();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	void				forceStartIdleMode();
	void				onAppEvent(const ds::Event&);


	// Data acquisition
	Globals				mGlobals;

	//Idle state of the app to detect state change
	bool				mIdling;

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_PDF_LEAK_TESTER_APP_H_
