#include "stdafx.h"

#include "settings_rewrite_app.h"

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

#include "cfg/settings_manager.h"
#include "cfg/settings_updater.h"

namespace downstream {

settings_rewrite_app::settings_rewrite_app()
	: ds::App(ds::RootList()

	// Note: this is where you'll customize the root list
	.ortho()
	.pickColor()


	)
	, mGlobals(mEngine, mAllData)
	, mQueryHandler(mEngine, mAllData)
	, mEventClient(mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mStoryView(nullptr)
{

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(StoryDataUpdatedEvent::NAME(), [this]()->ds::Event*{return new StoryDataUpdatedEvent(); });
	ds::event::Registry::get().addEventCreator(RequestAppExitEvent::NAME(), [this]()->ds::Event*{return new RequestAppExitEvent(); });
}

void settings_rewrite_app::setupServer(){

	mQueryHandler.runInitialQueries(true);

	/*

	ds::cfg::SettingsUpdater updater(mEngine);
	updater.updateSettings(ds::Environment::expand("%APP%/settings/engine.xml"), ds::Environment::expand("%APP%/settings/engine_updated.xml"));



	ds::cfg::Settings sm;
	sm.readFrom(ds::Environment::expand("%APP%/settings/engine_new.xml"), true);
	sm.readFrom(ds::Environment::expand("%APP%/settings/engine_new_override.xml"), true);
	std::string serverConnect = sm.getString("server:connect");


	ds::cfg::Settings::Setting newSetting;
	newSetting.mName = "holy:fuck_balls";
	newSetting.mType = "string";
	newSetting.mRawValue = "well crap on a stick";
	newSetting.mComment = "Testing!";
	sm.addSetting(newSetting);


	auto& adjustASetting = sm.getSetting("story:area", 1);
	adjustASetting.mRawValue = "whoop de doo";

	//sm.printAllSettings();

	//sm.writeTo(ds::Environment::expand("%APP%/settings/test_write.xml"));

	*/

	const bool cacheXML = mEngine.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	mStoryView = new StoryView(mGlobals);
	rootSprite.addChildPtr(mStoryView);

}

void settings_rewrite_app::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestAppExitEvent::WHAT()){
		mEngine.getRootSprite().callAfterDelay([this]{	quit(); }, 0.1f);
	} 
}

void settings_rewrite_app::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
}

void settings_rewrite_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){

		std::string thePath = (*it).string();

		bool autoDetect = true;
		bool includeComments = true;
		if(mStoryView) {
			includeComments = mStoryView->getIncludeComments();
			autoDetect = mStoryView->getIsEngineMode();
		}
		ds::cfg::SettingsUpdater updater(mEngine);
		if(autoDetect){
			if(thePath.find("engine.xml") != std::string::npos){
				updater.updateEngineSettings(thePath);
			} else {
				updater.updateSettings(thePath, thePath, includeComments);
			}
		} else {
			updater.updateSettings(thePath, thePath, includeComments);
		}
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::settings_rewrite_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
