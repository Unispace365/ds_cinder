#ifndef _MEDIAVIEWER_APP_H_
#define _MEDIAVIEWER_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"
#include "ds/ui/menu/touch_menu.h"

namespace mv {
class AllData;

class MediaViewer : public ds::App {
public:
	MediaViewer();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);

	virtual void		onTouchesBegan(ci::app::TouchEvent te);
	virtual void		onTouchesMoved(ci::app::TouchEvent te);
	virtual void		onTouchesEnded(ci::app::TouchEvent te);

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

	void				moveCamera(const ci::Vec3f& deltaMove);
	void				touchEventToTouchInfo(ci::app::TouchEvent& te, ds::ui::TouchInfo::Phase phasey);
};

} // !namespace mv

#endif // !_MEDIAVIEWER_APP_H_