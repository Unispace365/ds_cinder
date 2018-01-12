#ifndef _GENERIC_DATA_MODEL_APP_H_
#define _GENERIC_DATA_MODEL_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/touch/touch_debug.h>

#include "app/globals.h"
#include "query/data_wrangler.h"
#include "query/query_handler.h"

namespace downstream {
class AllData;

class generic_data_model_app : public ds::App {
public:
	generic_data_model_app();

	virtual void		mouseDown(ci::app::MouseEvent e) override;
	virtual void		mouseDrag(ci::app::MouseEvent e) override;
	virtual void		mouseUp(ci::app::MouseEvent e) override;
	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	void				forceStartIdleMode();
	void				onAppEvent(const ds::Event&);

	// Data
	AllData				mAllData;


	DataWrangler		mDataWrangler;

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

#endif // !_GENERIC_DATA_MODEL_APP_H_
