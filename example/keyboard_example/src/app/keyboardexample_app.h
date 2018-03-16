#ifndef _KEYBOARDEXAMPLE_APP_H_
#define _KEYBOARDEXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>

#include "app/globals.h"
#include "query/query_handler.h"
#include <ds/ui/soft_keyboard/soft_keyboard.h>

namespace example {
class AllData;

class KeyboardExample : public ds::App {
public:
	KeyboardExample();

	virtual void		onKeyDown(ci::app::KeyEvent event) override;
	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event);
private:
	// Data
	AllData				mAllData;

	// Data acquisition
	Globals				mGlobals;
	QueryHandler		mQueryHandler;

	ds::ui::SoftKeyboard*	mSoftKeyboard;

};

} // !namespace example

#endif // !_KEYBOARDEXAMPLE_APP_H_