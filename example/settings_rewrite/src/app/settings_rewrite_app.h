#ifndef _SETTINGS_REWRITE_APP_H_
#define _SETTINGS_REWRITE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/touch/touch_debug.h>

#include "app/globals.h"
#include "query/query_handler.h"

namespace downstream {
class AllData;

class settings_rewrite_app : public ds::App {
public:
	settings_rewrite_app();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event);

private:

	void				forceStartIdleMode();
	void				onAppEvent(const ds::Event&);

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	//Idle state of the app to detect state change
	bool				mIdling;

	// Handle mouse events and simulate touch events
	ds::TouchDebug		mTouchDebug;

	// App events can be handled here
	ds::EventClient		mEventClient;
};

} // !namespace downstream

#endif // !_SETTINGS_REWRITE_APP_H_
