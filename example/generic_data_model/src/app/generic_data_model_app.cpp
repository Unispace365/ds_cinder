#include "stdafx.h"

#include "generic_data_model_app.h"

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

#include "ui/story/story_view.h"
#include "ui/table/table_view.h"

namespace downstream {

generic_data_model_app::generic_data_model_app()
	: ds::App()
	, mGlobals(mEngine, mDataWrangler)
	, mDataWrangler(mEngine)
	, mEventClient(mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{



	/*

	ds::model::DataModelRef dmr("root", 1);
	dmr.addChild("tiles", ds::model::DataModelRef("tiles", 1));
	dmr.addChild("Blurps", ds::model::DataModelRef("tiles", 1));

	auto theTiles = dmr.getChild("tiles");
	theTiles.addChild("tile", ds::model::DataModelRef("hello", 2));
	theTiles.addChild("tile", ds::model::DataModelRef("is it me", 3));
	theTiles.addChild("tile", ds::model::DataModelRef("you're looking for", 4));

	dmr.printTree(true, "");
	*/


	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(DataUpdatedEvent::NAME(), [this]()->ds::Event*{return new DataUpdatedEvent(); });
	ds::event::Registry::get().addEventCreator(RequestAppExitEvent::NAME(), [this]()->ds::Event*{return new RequestAppExitEvent(); });

}

void generic_data_model_app::setupServer(){

	mDataWrangler.runQuery(true);

	const bool cacheXML = mEngine.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	//rootSprite.addChildPtr(new StoryView(mGlobals));
	rootSprite.addChildPtr(new TableView(mGlobals));
}

void generic_data_model_app::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestAppExitEvent::WHAT()){
		quit();
	} 
}

void generic_data_model_app::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

}


void generic_data_model_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::generic_data_model_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
