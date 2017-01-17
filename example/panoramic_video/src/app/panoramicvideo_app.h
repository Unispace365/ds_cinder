#ifndef _PANORAMICVIDEO_APP_H_
#define _PANORAMICVIDEO_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"
#include "ds/ui/menu/touch_menu.h"

namespace panoramic {
class AllData;

class PanoramicVideo : public ds::App {
public:
	PanoramicVideo();

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
	ds::ui::TouchMenu*	mTouchMenu;


	void				moveCamera(const ci::vec3& deltaMove);
};

} // !namespace panoramic

#endif // !_PANORAMICVIDEO_APP_H_

