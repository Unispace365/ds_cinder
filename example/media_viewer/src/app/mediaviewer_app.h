#ifndef _MEDIAVIEWER_APP_H_
#define _MEDIAVIEWER_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/AppBase.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"
#include "ds/ui/menu/touch_menu.h"

#include <ds/ui/sprite/gst_video.h>

namespace mv {
class AllData;

class MediaViewer : public ds::App {
public:
	MediaViewer();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

	void				fileDrop(ci::app::FileDropEvent event) override;
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

	ds::ui::Sprite*		mStreamerParent;
	ds::ui::GstVideo*	mStreamer;

	void				touchEventToTouchInfo(ds::ui::TouchEvent& te, ds::ui::TouchInfo::Phase phasey);
};

} // !namespace mv

#endif // !_MEDIAVIEWER_APP_H_

