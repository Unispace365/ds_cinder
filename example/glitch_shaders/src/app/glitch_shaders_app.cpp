#include "stdafx.h"

#include "glitch_shaders_app.h"

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

#include "ui/story/story_view.h"

namespace downstream {

glitch_shaders_app::glitch_shaders_app()
  : ds::App()
  , mGlobals(mEngine, mAllData)
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

void glitch_shaders_app::setupServer() {

	// NOTES!
	// Drop a video onto the app to see that with glitch shaders
	// Or see app_settings for how to use your webcam
	// Tap the app to advance through shaders
	// Drag to change the effect

	const bool cacheXML = mEngine.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);


	ds::ui::Sprite& rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	// add sprites
	rootSprite.addChildPtr(new StoryView(mGlobals));
}
void glitch_shaders_app::onAppEvent(const ds::Event& in_e) {
	if (in_e.mWhat == RequestAppExitEvent::WHAT()) {
		quit();
	}
}

void glitch_shaders_app::onKeyDown(ci::app::KeyEvent event) {
	using ci::app::KeyEvent;
}

void glitch_shaders_app::fileDrop(ci::app::FileDropEvent event) {
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {

		mAllData.mStories.clear();

		ds::model::StoryRef newStory;
		newStory.setPrimaryResource(ds::Resource((*it).string()));
		mAllData.mStories.emplace_back(newStory);
		mEngine.getNotifier().notify(StoryDataUpdatedEvent());
		break;

		//		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		//	mv->initialize();
		//	mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::glitch_shaders_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
