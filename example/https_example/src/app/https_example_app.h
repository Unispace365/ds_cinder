#ifndef _HTTPS_EXAMPLE_APP_H_
#define _HTTPS_EXAMPLE_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"
#include "ds/network/https_client.h"

namespace example {
class AllData;

class https_example : public ds::App {
public:
	https_example();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	typedef ds::App		inherited;

	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	//Idle state of the app to detect state change
	bool				mIdling;


	ds::TouchDebug		mTouchDebug;

	ds::net::HttpsRequest	mHttpsRequest;

	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace example

#endif // !_HTTPS_EXAMPLE_APP_H_