#include "mediaviewer_app.h"

#include <Poco/String.h>
#include <Poco/File.h>
#include <Poco/Path.h>

#include <cinder/Rand.h> 

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/util/string_util.h>
#include <ds/ui/media/media_viewer.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ui/viewers/viewer_controller.h"


namespace mv {

MediaViewer::MediaViewer()
	: inherited()
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mTouchMenu(nullptr)
	, mStreamer(nullptr)
	, mStreamerParent(nullptr)
{

	//mLastFilePath = "D:/content/sample_videos_2/51abba233fb16.mp4";
	mLastFilePath = "D:/content/sample_videos_2/06_campus_09_26_4k.mp4";


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
	mEngine.editFonts().registerFont("Noto Sans", "noto-thin");


	registerKeyPress("Toggle NV decode", [this] { mNVDecode = !mNVDecode; std::cout << "Nvidia decode is " << mNVDecode << std::endl; }, ci::app::KeyEvent::KEY_COMMA);
	registerKeyPress("Toggle GL Mode", [this] { mGlMode = !mGlMode; std::cout << "GL Mode is " << mGlMode << std::endl; }, ci::app::KeyEvent::KEY_PERIOD);
}

void MediaViewer::setupServer(){

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mStreamer = nullptr;
	mGlobals.initialize();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	rootSprite.setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	
	// add sprites
	rootSprite.addChildPtr(new ViewerController(mGlobals));

	/*
	*/

	mStreamerParent = new ds::ui::Sprite(mEngine);
	rootSprite.addChildPtr(mStreamerParent);

	mTouchMenu = new ds::ui::TouchMenu(mEngine);
	rootSprite.addChildPtr(mTouchMenu);

	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage = "%APP%/data/images/menu/TouchMenuBlur.png";
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mClusterRadius = 250.0f;
	tmc.mBackgroundOpacity = 0.7f;
	mTouchMenu->setMenuConfig(tmc);

	std::vector<ds::ui::TouchMenu::MenuItemModel> menuItemModels;
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Exit", "%APP%/data/images/menu/exit_app_normal.png", "%APP%/data/images/menu/exit_app_glow.png", [this](ci::vec3){ ci::app::App::quit(); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Close All", "%APP%/data/images/menu/close_normal.png", "%APP%/data/images/menu/close_glow.png", [this](ci::vec3){ mEngine.getNotifier().notify(RequestCloseAllEvent()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Search", "%APP%/data/images/menu/search_normal.png", "%APP%/data/images/menu/search_glow.png", [this](ci::vec3 pos){  }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Layout", "%APP%/data/images/menu/pinboard_normal.png", "%APP%/data/images/menu/pinboard_glow.png", [this](ci::vec3){ mEngine.getNotifier().notify(RequestLayoutEvent()); }));


	mTouchMenu->setMenuItemModels(menuItemModels);

	mEngine.setTouchInfoPipeCallback([this](const ds::ui::TouchInfo& ti){ mTouchMenu->handleTouchInfo(ti); });

}



void MediaViewer::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	if(event.getCode() == KeyEvent::KEY_v && event.isControlDown()){
		auto fileNameOrig = ds::Environment::getClipboard();
		ds::model::MediaRef newMedia = ds::model::MediaRef();

		Poco::File filey = Poco::File(fileNameOrig);
		std::string extensionay = Poco::Path(filey.path()).getExtension();
		std::transform(extensionay.begin(), extensionay.end(), extensionay.begin(), ::tolower);
		if(extensionay.find("png") != std::string::npos
		   || extensionay.find("jpg") != std::string::npos
		   || extensionay.find("jpeg") != std::string::npos
		   || extensionay.find("gif") != std::string::npos
		   )
		{
			Poco::Path pathy = filey.path();
			std::string fileName = pathy.getFileName();
			fileName = fileName.substr(0, fileName.length() - 4);
			newMedia.setPrimaryResource(ds::Resource(fileNameOrig, ds::Resource::IMAGE_TYPE));
			newMedia.setTitle(ds::wstr_from_utf8(fileName));
			newMedia.setBody(ds::wstr_from_utf8(fileNameOrig));
		}
		else {
			newMedia.setPrimaryResource(ds::Resource(fileNameOrig, ds::Resource::parseTypeFromFilename(fileNameOrig)));
			newMedia.setTitle(L"The World Wide Web Internet Browser");
			newMedia.setBody(ds::wstr_from_utf8(fileNameOrig));
		}
		mEngine.getNotifier().notify(RequestMediaOpenEvent(newMedia, ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f), 600.0f));


	} else if(event.getCode() == KeyEvent::KEY_k){
		if(mStreamer){
			mStreamer->stop();
			mStreamer->release();
			mStreamer = nullptr;
		}
		mStreamer = new ds::ui::GstVideo(mEngine);
		mStreamer->startStream(mGlobals.getSettingsLayout().getString("streaming:pipeline", 0, ""),
							 (float) mGlobals.getSettingsLayout().getInt("streaming:width", 0, 640), 
							 (float) mGlobals.getSettingsLayout().getInt("streaming:height", 0, 480));
		mStreamerParent->addChildPtr(mStreamer);
		mStreamer->enable(true);
		mStreamer->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
		
	//	mStreamerParent->addChildPtr(new ds::ui::Image(mEngine, "%APP%/data/images/sample.png"));
	}
}

void MediaViewer::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	ci::vec3 locationy = ci::vec3((float)event.getX(), (float)event.getY(), 0.0f);
	float incrementy = 50.0f;
	float startWidth = mEngine.getWorldWidth() / 4.0f;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());

// 		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
// 		mv->initializeIfNeeded();
// 		mEngine.getRootSprite().addChildPtr(mv);

		Poco::File filey = Poco::File((*it).string());
		Poco::Path pathy = filey.path();
		std::string fileName = pathy.getFileName();
		fileName = fileName.substr(0, fileName.length() - 4);

		mLastFilePath = (*it).string();

		ds::model::MediaRef newMedia = ds::model::MediaRef();
		newMedia.setPrimaryResource(ds::Resource((*it).string(), ds::Resource::parseTypeFromFilename((*it).string())));
		newMedia.setTitle(ds::wstr_from_utf8(fileName));
		newMedia.setBody((*it).wstring());
		mEngine.getNotifier().notify(RequestMediaOpenEvent(newMedia, ci::vec3(locationy.x - startWidth/2.0f, locationy.y - startWidth/2.0f, 0.0f), startWidth, mGlMode, mNVDecode));
		locationy.x += incrementy;
		locationy.y += incrementy;
	}

	event.setHandled();
}

} // namespace mv

// This line tells Cinder to actually create the application
CINDER_APP(mv::MediaViewer, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })

