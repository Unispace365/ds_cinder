#include "stdafx.h"

#include "full_starter_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/story/story_controller.h"

namespace fullstarter {

FullStarterApp::FullStarterApp()
  : ds::App()
  , mEventClient(mEngine) {

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(SomethingHappenedEvent::NAME(),
											   []() -> ds::Event* { return new SomethingHappenedEvent(); });

	mEventClient.listenToEvents<SomethingHappenedEvent>(
		[](const SomethingHappenedEvent& e) { std::cout << "Something happened" << std::endl; });

	// registerKeyPress("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); },
	// ci::app::KeyEvent::KEY_n);
}

void FullStarterApp::setupServer() {
	// add sprites
	mEngine.getRootSprite().addChildPtr(new StoryController(mEngine));

	// For this test app, we show the app to start with for simplicity
	// In a real scenario, you'll probably want to start idled / attracting
	mEngine.stopIdling();
}

void FullStarterApp::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace fullstarter

// This line tells Cinder to actually create the application
CINDER_APP(fullstarter::FullStarterApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
