#include "drag_destination_example_app.h"

#include <Poco/String.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/story/story_view.h"

namespace example {

drag_destination_example::drag_destination_example()
  : ds::App()
  , mGlobals(mEngine, mAllData)
  , mQueryHandler(mEngine, mAllData) {

	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
}

void drag_destination_example::setupServer() {

	// ---------------------------- HELLLOOOOOOOOOOOOO -----------------------------//
	// If you came here looking for info on how to do a drag destination,
	// Look in story_view.cpp. It has all the sample code you need
	// ---------------------------------------------------------------------------- //

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();

	ds::ui::Sprite& rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	// add sprites
	rootSprite.addChildPtr(new StoryView(mGlobals));
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::drag_destination_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))