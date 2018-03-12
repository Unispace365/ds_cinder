#include "stdafx.h"

#include "physics_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/globals.h"

#include "events/app_events.h"

#include "ui/bouncy/bouncy_view.h"

namespace physics {

physics_example_app::physics_example_app()
	: ds::App(ds::RootList())
	, mGlobals(mEngine, mAllData)
	, mQueryHandler(mEngine, mAllData)
	, mEventClient(mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(StoryDataUpdatedEvent::NAME(), [this]()->ds::Event*{return new StoryDataUpdatedEvent(); });

}

void physics_example_app::setupServer(){


	mQueryHandler.runInitialQueries(true);

	const bool cacheXML = mEngine.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	rootSprite.addChildPtr(new BouncyView(mGlobals));
}


void physics_example_app::onAppEvent(const ds::Event& in_e){
}

void physics_example_app::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

}

void physics_example_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace physics

// This line tells Cinder to actually create the application
CINDER_APP(physics::physics_example_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
