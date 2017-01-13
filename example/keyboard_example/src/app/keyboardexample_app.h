#ifndef _KEYBOARDEXAMPLE_APP_H_
#define _KEYBOARDEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"

namespace example {
class AllData;

class KeyboardExample : public ds::App {
public:
	KeyboardExample();

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


	void				moveCamera(const ci::vec3& deltaMove);
};

} // !namespace example

#endif // !_KEYBOARDEXAMPLE_APP_H_