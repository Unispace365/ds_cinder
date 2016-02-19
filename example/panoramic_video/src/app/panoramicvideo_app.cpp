#include "panoramicvideo_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/video_list.h"
#include "ui/panoramic_view.h"

namespace panoramic {

PanoramicVideo::PanoramicVideo()
	: inherited(ds::RootList()

	// Note: this is where you'll customize the root list
								.ortho() 
								.pickColor()

								.persp() 
								.perspFov(60.0f)
								.perspPosition(ci::Vec3f(0.0, 0.0f, 10.0f))
								.perspTarget(ci::Vec3f(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mTouchMenu(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-thin");

	enableCommonKeystrokes(true);
}

void PanoramicVideo::setupServer(){


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
			p.mTarget = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));


	rootSprite.addChildPtr(new PanoramicView(mGlobals));
	rootSprite.addChildPtr(new VideoList(mGlobals));

	mTouchMenu = new ds::ui::TouchMenu(mEngine);
	rootSprite.addChildPtr(mTouchMenu);

	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage = "%APP%/data/images/menu/TouchMenuBlur.png";
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mClusterRadius = 250.0f;
	tmc.mBackgroundOpacity = 0.7f;
	mTouchMenu->setMenuConfig(tmc);

	std::vector<ds::ui::TouchMenu::MenuItemModel> menuItemModels;
//	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Exit", "%APP%/data/images/menu/exit_app_normal.png", "%APP%/data/images/menu/exit_app_glow.png", [this](ci::Vec3f){ std::exit(0); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Close All", "%APP%/data/images/menu/close_normal.png", "%APP%/data/images/menu/close_glow.png", [this](ci::Vec3f){ mEngine.getNotifier().notify(RequestCloseAllEvent()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Show Video List", "%APP%/data/images/menu/pinboard_normal.png", "%APP%/data/images/menu/pinboard_glow.png", [this](ci::Vec3f p){ mEngine.getNotifier().notify(RequestVideoList(p)); }));

	mTouchMenu->setMenuItemModels(menuItemModels);

	mEngine.setTouchInfoPipeCallback([this](const ds::ui::TouchInfo& ti){ mTouchMenu->handleTouchInfo(ti); });
}

void PanoramicVideo::update() {
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

void PanoramicVideo::keyDown(ci::app::KeyEvent event){
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

void PanoramicVideo::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void PanoramicVideo::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void PanoramicVideo::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void PanoramicVideo::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());

		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace panoramic

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(panoramic::PanoramicVideo, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))