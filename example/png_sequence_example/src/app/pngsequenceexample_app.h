#ifndef _PNGSEQUENCEEXAMPLE_APP_H_
#define _PNGSEQUENCEEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include <ds/ui/sprite/png_sequence_sprite.h>

#include "app/globals.h"
#include "ds/touch/touch_debug.h"

namespace example {

/**  PngSequenceExample
	Drop a series of png images onto the app window to play them back. Simple!
*/
class PngSequenceExample : public ds::App {
public:
	PngSequenceExample();

	virtual void		mouseDown(ci::app::MouseEvent e);
	virtual void		mouseDrag(ci::app::MouseEvent e);
	virtual void		mouseUp(ci::app::MouseEvent e);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	typedef ds::App		inherited;


	// Data acquisition
	Globals						mGlobals;

	ds::ui::PngSequenceSprite*	mPngSequence;

	ds::TouchDebug				mTouchDebug;
};

} // !namespace example

#endif // !_PNGSEQUENCEEXAMPLE_APP_H_