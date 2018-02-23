#include "panoramicvideo_app.h"

//#include <Poco/String.h>

#include <ds/app/environment.h>
#include <ds/app/engine/engine.h>

//#include <ds/ui/media/media_viewer.h>


#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/video_list.h"
#include "ui/panoramic_view.h"
//#include <ds/debug/logger.h>

namespace panoramic {

PanoramicVideo::PanoramicVideo()
	: inherited() 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mTouchMenu(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
	mEngine.editFonts().registerFont("Noto Sans", "noto-thin");
}

void PanoramicVideo::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();


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
//	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Exit", "%APP%/data/images/menu/exit_app_normal.png", "%APP%/data/images/menu/exit_app_glow.png", [this](ci::vec3){ std::exit(0); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Close All", "%APP%/data/images/menu/close_normal.png", "%APP%/data/images/menu/close_glow.png", [this](ci::vec3){ mEngine.getNotifier().notify(RequestCloseAllEvent()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Show Video List", "%APP%/data/images/menu/pinboard_normal.png", "%APP%/data/images/menu/pinboard_glow.png", [this](ci::vec3 p){ mEngine.getNotifier().notify(RequestVideoList(p)); }));

	mTouchMenu->setMenuItemModels(menuItemModels);

	mEngine.setTouchInfoPipeCallback([this](const ds::ui::TouchInfo& ti){ mTouchMenu->handleTouchInfo(ti); });
}

} // namespace panoramic

// This line tells Cinder to actually create the application
CINDER_APP(panoramic::PanoramicVideo, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))

