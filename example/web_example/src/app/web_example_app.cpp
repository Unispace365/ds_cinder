#include "web_example_app.h"

#include <cinder/Clipboard.h>

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "ui/web/web_view.h"

namespace web_example {

web_example::web_example()
	: inherited(ds::RootList().ortho() ) 
	, mGlobals(mEngine)
	, mWebView(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/FONT_FILE_HERE.ttf"), "font-name-here");

	enableCommonKeystrokes(true);
}

void web_example::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.2f, 0.1f, 0.6f));
	// add sprites

	mWebView = new WebView(mGlobals);
	rootSprite.addChild(*mWebView);
}

void web_example::update() {
	inherited::update();

}

void web_example::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);

	if(event.getChar() == KeyEvent::KEY_v && event.isControlDown()){
		if(ci::Clipboard::hasString() && mWebView){
			mWebView->setUrl(ci::Clipboard::getString());
		}
	} else if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	} else if(event.getChar() == KeyEvent::KEY_b){
		if(mWebView){
			mWebView->goBack();
		}
	} else if(event.getChar() == KeyEvent::KEY_f){
		if(mWebView){
			mWebView->goForward();
		}
	} else if(event.getChar() == KeyEvent::KEY_l){
		if(mWebView){
			mWebView->reload();
		}
	}
}

} // namespace web_example

// This line tells Cinder to actually create the application
CINDER_APP(web_example::web_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))