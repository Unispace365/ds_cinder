#ifndef _PANGO_APP_H_
#define _PANGO_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/touch/touch_debug.h>

#include "app/globals.h"
#include "query/query_handler.h"


namespace pango {
class AllData;

class PangoApp : public ds::App {
public:
	PangoApp();

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

} // !namespace pango

#endif // !_PANGO_APP_H_