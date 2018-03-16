#ifndef _PNGSEQUENCEEXAMPLE_APP_H_
#define _PNGSEQUENCEEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include <ds/ui/sprite/png_sequence_sprite.h>

#include "app/globals.h"

namespace example {

/**  PngSequenceExample
	Drop a series of png images onto the app window to play them back. Simple!
*/
class PngSequenceExample : public ds::App {
public:
	PngSequenceExample();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	typedef ds::App		inherited;


	// Data acquisition
	Globals						mGlobals;

	ds::ui::PngSequenceSprite*	mPngSequence;
};

} // !namespace example

#endif // !_PNGSEQUENCEEXAMPLE_APP_H_

