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

#include "app/app_defs.h"
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
	, mIdling(false)
	, mTouchDebug(mEngine)
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

	// Fonts links together a font name and a physical font file
	// Then the "text.xml" and TextCfg will use those font names to specify visible settings (size, color, leading)
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& setting){
		mEngine.editFonts().installFont(ds::Environment::expand(setting.mRawValue), setting.mName);
	});

	// Colors
	// After registration, colors can be called by name from settings files or in the app
	mEngine.editColors().clear();
	mEngine.editColors().install(ci::Color(1.0f, 1.0f, 1.0f), "white");
	mEngine.editColors().install(ci::Color(0.0f, 0.0f, 0.0f), "black");
	mEngine.loadSettings("COLORS", "colors.xml");
	mEngine.getSettings("COLORS").forEachSetting([this](const ds::cfg::Settings::Setting& setting){
		mEngine.editColors().install(setting.getColorA(mEngine), setting.mName);
	}, ds::cfg::SETTING_TYPE_COLOR);

	/* Settings */
	mEngine.loadSettings(SETTINGS_APP, "app_settings.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
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

	const bool cacheXML = mGlobals.getAppSettings().getBool("xml:cache", 0, true);
	ds::ui::XmlImporter::setAutoCache(cacheXML);

	const size_t numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(size_t i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	mStoryView = new StoryView(mGlobals);
	rootSprite.addChildPtr(mStoryView);

	// The engine will actually be idling, and this gets picked up on the next update
	mIdling = false;
}

void settings_rewrite_app::update() {
	ds::App::update();

	bool rootsIdle = true;
	const size_t numRoots = mEngine.getRootCount();
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;
		if(!mEngine.getRootSprite(i).isIdling()){
			rootsIdle = false;
			break;
		}
	}

	if(rootsIdle && !mIdling){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify(IdleStartedEvent());
		

	} else if(!rootsIdle && mIdling){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify(IdleEndedEvent());
	}

}

void settings_rewrite_app::forceStartIdleMode(){
	// force idle mode to start again
	const size_t numRoots = mEngine.getRootCount();
	for(size_t i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;
		mEngine.getRootSprite(i).startIdling();
	}
	mEngine.startIdling();
	mIdling = true;

	mEngine.getNotifier().notify(IdleStartedEvent());
}

void settings_rewrite_app::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestAppExitEvent::WHAT()){
		mEngine.getRootSprite().callAfterDelay([this]{	quit(); }, 0.1f);
	} 
}

void settings_rewrite_app::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	if(event.getCode() == KeyEvent::KEY_i){
		forceStartIdleMode();
	}
}

void settings_rewrite_app::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void settings_rewrite_app::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void settings_rewrite_app::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void settings_rewrite_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){

		std::string thePath = (*it).string();

		ds::cfg::SettingsUpdater updater(mEngine);
		if(mStoryView && mStoryView->getIsEngineMode()){
			updater.updateEngineSettings(thePath);
		} else {
			updater.updateSettings(thePath, thePath);
		}
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::settings_rewrite_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
