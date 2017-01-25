#include "cefdevelop_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/app/App.h>
#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>


#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/story/story_view.h"

#include <ds/ui/sprite/web.h>

namespace cef {

CefDevelop::CefDevelop()
	: inherited(ds::RootList()

	// Note: this is where you'll customize the root list
								.ortho() 
								.pickColor()

								.persp() 
								.perspFov(60.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 10.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mWebby(nullptr)
{

	enableCommonKeystrokes(true);
}

void CefDevelop::setupServer(){

	/*fonts in use */
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachTextKey([this](const std::string& key){
		mEngine.editFonts().registerFont(ds::Environment::expand(mEngine.getSettings("FONTS").getText(key)), key);
	});

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();

	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
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
	rootSprite.addChildPtr(new StoryView(mGlobals));

	rootSprite.enable(true);
	rootSprite.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);

	/*
	auto webby = new ds::ui::Web(mGlobals.mEngine, 1920.0f, 1080.0f);
	webby->loadUrl("https://google.com");
	rootSprite.addChildPtr(webby);
	webby->setCenter(0.5f, 0.5f, 0.5f);
	webby->setPosition(webby->getWidth()/2.0f, webby->getHeight() / 2.0f);
	ci::randSeed(26987);
	webby->setRotation(ci::vec3(ci::randFloat(0.0f, 360.0f), ci::randFloat(0.0f, 360.0f), ci::randFloat(0.0f, 360.0f)));
	webby->enable(true);
	webby->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
	webby->setTouchScaleMode(true);
	mWebby = webby;
	*/

	rootSprite.setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		/*
		ds::ui::WebPlayer* wp = new ds::ui::WebPlayer(mEngine, true);
		wp->setWebViewSize(ci::vec2(1366, 1366.0f));
		wp->setMedia("https://google.com"); 
		bs->addChildPtr(wp);
		mWebby = wp->getWeb();
		wp->enable(true);
		wp->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
		wp->setTouchScaleMode(true);
		//mWebby->enable(true);

		*/



		//const std::string urly = "downstream.com";
	//	const std::string urly = "file://D:/content/sample_videos_2/vp9_4k.webm";
	//	const std::string urly = "file://D:/test_pdfs/BPS C06_CIM_Services.pdf";
		//const std::string urly = "google.com";
	//	const std::string urly = "http://i.imgur.com/r6sS64A.gifv";
			const std::string urly = "https://google.com";
	//	const std::string urly = "https://duapps.deloitte.com/BusinessMetricsDashboard/";
	//	const std::string urly = "http://www.google.com/doodles/30th-anniversary-of-pac-man";
	//	const std::string urly = "http://www.capitolconnect.com/boeing/reports/default.asp?user=boeingyes";
		//const std::string urly = "https://drive.google.com/drive/my-drive";
		//const std::string urly = "https://agoing.agsafoin.com/"; // generates an error cause the site can't be reached
		auto webby = new ds::ui::Web(mGlobals.mEngine, 1920.0f, 1080.0f);
		webby->loadUrl(urly);
		bs->addChildPtr(webby);
		//webby->setCenter(0.5f, 0.5f, 0.5f);
		//webby->setRotation(ci::vec3(ci::randFloat(0.0f, 360.0f), ci::randFloat(0.0f, 360.0f), ci::randFloat(0.0f, 360.0f)));
		webby->enable(true);
	//	webby->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
		webby->setTouchScaleMode(true);
		mWebby = webby;

		webby->setFullscreenChangedCallback([this, webby](const bool isFullscreen){
			if(isFullscreen){
				webby->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
			} else {
				webby->setSize(1920.0f, 1080.0f);
			}
		});

		//bs->enable(false);

		// This tests the "orphans" code in the web handler. Basically, double-click to load a web sprite
		static int starty = 0;
		if(starty % 2 == 0){
			webby->release();
			mWebby = nullptr;
		}

		starty++;
	});


	//auto webby2 = new ds::ui::Web(mGlobals.mEngine, 1920.0f, 1080.0f);
	//webby2->loadUrl("https://google.com");
	//rootSprite.addChildPtr(webby2);

}


void CefDevelop::update() {
	inherited::update();

	if( mEngine.isIdling() && !mIdling ){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify( IdleStartedEvent() );
	} else if ( !mEngine.isIdling() && mIdling ){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify( IdleEndedEvent() );
	}

}

void CefDevelop::keyUp(ci::app::KeyEvent event){
	if(mWebby){
		mWebby->sendKeyUpEvent(event);
	}
}

void CefDevelop::keyDown(ci::app::KeyEvent event){
	if(event.getChar() == '+' && mWebby){
		mWebby->setZoom(mWebby->getZoom() + 0.1);
	} else if(event.getChar() == '-' && mWebby){
		mWebby->setZoom(mWebby->getZoom() - 0.1);
	} else if(event.getChar() == '0'){
		std::cout << mWebby->getZoom() << std::endl;
	}

	if(event.getChar() == 't'){
		mWebby->setWebTransparent(!mWebby->getWebTransparent());
		std::cout << "Web is now " << mWebby->getWebTransparent() << " transparent" << std::endl;
	}

	if(mWebby){
		mWebby->sendKeyDownEvent(event);
		return;
	}
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();

	// Shows all enabled sprites with a label for class type
	} else if(event.getCode() == KeyEvent::KEY_f){

		const int numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(int i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
					sprite.setTransparent(false);
					sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
					sprite.setOpacity(0.95f);

					ds::ui::Text* labelly = mGlobals.getText("media_viewer:title").create(mEngine, &sprite);
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	}
}

void CefDevelop::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void CefDevelop::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void CefDevelop::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void CefDevelop::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());

		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace cef

// This line tells Cinder to actually create the application
CINDER_APP(cef::CefDevelop, cinder::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))