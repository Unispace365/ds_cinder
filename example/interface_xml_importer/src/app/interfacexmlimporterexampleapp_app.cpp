#include "interfacexmlimporterexampleapp_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "view/sample_view.h"

namespace importer_example {

InterfaceXmlImporterExampleApp::InterfaceXmlImporterExampleApp()
	: inherited()
	, mGlobals(mEngine)
{


	/*fonts in use */
	// These fonts need to be registered to be picked up by the 'text.xml' file.
	// Then in the css, the config name can be picked up and look this up. a long chain for 'convenience'
	mEngine.editFonts().install("Noto Sans Bold", "sample-light");

	enableCommonKeystrokes(true);
}

void InterfaceXmlImporterExampleApp::setupServer(){



	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	mGlobals.reloadXml();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	// clear out old sprites (if this function was called from the 'r' key reload
	rootSprite.clearChildren();

	// add sprites
	rootSprite.addChild(*(new SampleView(mGlobals)));
}

void InterfaceXmlImporterExampleApp::update() {
	inherited::update();
}

void InterfaceXmlImporterExampleApp::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();
	}
}

} // namespace importer_example

// This line tells Cinder to actually create the application
CINDER_APP(importer_example::InterfaceXmlImporterExampleApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))