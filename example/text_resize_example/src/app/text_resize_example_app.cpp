#include "stdafx.h"

#include "text_resize_example_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/resize_example/text_resize_controller.h"

namespace text_resize {

text_resize_example_app::text_resize_example_app()
  : ds::App()
  , mEventClient(mEngine) {

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(SomethingHappenedEvent::NAME(),
											   [this]() -> ds::Event* { return new SomethingHappenedEvent(); });

	mEventClient.listenToEvents<SomethingHappenedEvent>(
		[this](const SomethingHappenedEvent& e) { std::cout << "Something happened" << std::endl; });

	// registerKeyPress("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); },
	// ci::app::KeyEvent::KEY_n);
}

void text_resize_example_app::setupServer() {
	// add sprites
	mEngine.getRootSprite().addChildPtr(new TextResizeController(mEngine));

	// For this test app, we show the app to start with for simplicity
	// In a real scenario, you'll probably want to start idled / attracting
	mEngine.stopIdling();
}

void text_resize_example_app::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace text_resize

// This line tells Cinder to actually create the application
CINDER_APP(text_resize::text_resize_example_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
