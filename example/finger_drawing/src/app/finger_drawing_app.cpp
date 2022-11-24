#include "finger_drawing_app.h"

#include <Poco/String.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h>
#include <cinder/app/RendererGl.h>

#include "app/globals.h"

#include "events/app_events.h"

#include "ui/story/drawing_view.h"

namespace example {

finger_drawing::finger_drawing()
  : ds::App()
  , mGlobals(mEngine, mAllData)
  , mQueryHandler(mEngine, mAllData)
  , mEventClient(mEngine.getNotifier(), [this](const ds::Event* m) {
	  if (m) this->onAppEvent(*m);
  }) {

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(StoryDataUpdatedEvent::NAME(),
											   [this]() -> ds::Event* { return new StoryDataUpdatedEvent(); });
	ds::event::Registry::get().addEventCreator(RequestAppExitEvent::NAME(),
											   [this]() -> ds::Event* { return new RequestAppExitEvent(); });
}

void finger_drawing::setupServer() {


	mQueryHandler.runInitialQueries(true);

	const bool cacheXML = mEngine.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);

	ds::ui::Sprite& rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	// add sprites
	rootSprite.addChildPtr(new DrawingView(mGlobals));
}


void finger_drawing::onAppEvent(const ds::Event& in_e) {
	if (in_e.mWhat == RequestAppExitEvent::WHAT()) {
		quit();
	}
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::finger_drawing, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })