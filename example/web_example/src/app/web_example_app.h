#ifndef _WEB_EXAMPLE_APP_H_
#define _WEB_EXAMPLE_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"

namespace web_example {
class WebView;

class web_example : public ds::App {
public:
	web_example();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;
	// Data acquisition
	Globals				mGlobals;

	WebView*			mWebView;


	void				moveCamera(const ci::Vec3f& deltaMove);
};

} // !namespace web_example

#endif // !_WEB_EXAMPLE_APP_H_