#ifndef _MEDIASLIDESHOW_APP_H_
#define _MEDIASLIDESHOW_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"

#include <ds/ui/media/media_slideshow.h>

namespace example {
class AllData;

class MediaSlideshow : public ds::App {
public:
	MediaSlideshow();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:

	// Data acquisition
	Globals				mGlobals;

	ds::ui::MediaSlideshow* mSlideshow;

};

} // !namespace example

#endif // !_MEDIASLIDESHOW_APP_H_

