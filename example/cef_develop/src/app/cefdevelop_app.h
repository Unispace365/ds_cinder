#ifndef _CEFDEVELOP_APP_H_
#define _CEFDEVELOP_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <cinder/app/RendererGl.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include "ds/touch/touch_debug.h"

#include <ds/ui/sprite/web.h>

namespace cef {
class AllData;

class CefDevelop : public ds::App {
public:
	CefDevelop();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		onKeyUp(ci::app::KeyEvent event) override;
	virtual void		onKeyDown(ci::app::KeyEvent event) override;
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

	ds::ui::Web*		mWebby;

	ds::TouchDebug		mTouchDebug;


	void				moveCamera(const ci::vec3& deltaMove);
};

} // !namespace cef

#endif // !_CEFDEVELOP_APP_H_