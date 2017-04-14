#ifndef _MEDIA_TESTER_APP_H_
#define _MEDIA_TESTER_APP_H_

#include <cinder/app/App.h>
#include <ds/ui/sprite/text.h>
#include <ds/app/app.h>

#include "globals.h"

namespace mv {

class ImageThreading : public ds::App {
public:
	ImageThreading();

	virtual void		fileDrop(ci::app::FileDropEvent event);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void update();
private:
	typedef ds::App		inherited;

	void				fitSpriteInArea(ci::Rectf area, ds::ui::Sprite* spriddy);
	ds::ui::Sprite*		mMedia;
	ds::ui::Text*		mLabelText;
	ds::ui::Sprite*		mHeader;
	bool				mIsVideo;

	Globals				mGlobals;
};

} // !namespace test

#endif // !_MEDIA_TESTER_APP_H_

