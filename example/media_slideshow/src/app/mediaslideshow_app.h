#ifndef _MEDIASLIDESHOW_APP_H_
#define _MEDIASLIDESHOW_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "ds/touch/touch_debug.h"

#include <ds/ui/media/media_slideshow.h>

namespace example {
class AllData;

class MediaSlideshow : public ds::App {
public:
	MediaSlideshow();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:

	// Data acquisition
	Globals				mGlobals;

	ds::TouchDebug		mTouchDebug;

	ds::ui::MediaSlideshow* mSlideshow;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace example

#endif // !_MEDIASLIDESHOW_APP_H_