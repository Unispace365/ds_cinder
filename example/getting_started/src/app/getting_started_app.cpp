#include "stdafx.h"

#include "getting_started_app.h"

#include <ds/app/engine/engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>
#include <ds/ui/menu/touch_menu.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/slides/slide_controller.h"

namespace downstream {

getting_started_app::getting_started_app()
	: ds::App()
	, mEventClient(mEngine)
{
	/// Initialize the current slide
	mEngine.mContent.setProperty("current_slide", 0);

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(SlideForwardRequest::NAME(), [this]()->ds::Event* {return new SlideForwardRequest(); });
	ds::event::Registry::get().addEventCreator(SlideBackRequest::NAME(), [this]()->ds::Event* {return new SlideBackRequest(); });
	ds::event::Registry::get().addEventCreator(SlideSetRequest::NAME(), [this]()->ds::Event* {return new SlideSetRequest(); });

	//mEventClient.listenToEvents<ds::DirectoryWatcher::Changed>([this](auto e) { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); });

}

void getting_started_app::setupServer(){
	// add sprites
	mEngine.getRootSprite().addChildPtr(new SlideController(mEngine));

	auto mTouchMenu = new ds::ui::TouchMenu(mEngine);
	mEngine.getRootSprite().addChildPtr(mTouchMenu);

	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage = "%APP%/data/images/menu/TouchMenuBlur.png";
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mClusterRadius = 250.0f;
	tmc.mBackgroundOpacity = 0.7f;
	mTouchMenu->setMenuConfig(tmc);

	std::vector<ds::ui::TouchMenu::MenuItemModel> menuItemModels;
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Exit", "%APP%/data/images/menu/exit_app_normal.png", "%APP%/data/images/menu/exit_app_glow.png", [this](ci::vec3) { mEngine.getNotifier().notify(ds::app::RequestAppExitEvent()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Next", "%APP%/data/images/menu/next_normal.png", "%APP%/data/images/menu/next_glow.png", [this](ci::vec3 pos) {mEngine.getNotifier().notify(SlideForwardRequest()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Home", "%APP%/data/images/menu/home_normal.png", "%APP%/data/images/menu/home_glow.png", [this](ci::vec3) { mEngine.mContent.setProperty("current_slide", 6); mEngine.getNotifier().notify(SlideSetRequest()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Prev", "%APP%/data/images/menu/prev_normal.png", "%APP%/data/images/menu/prev_glow.png", [this](ci::vec3) { mEngine.getNotifier().notify(SlideBackRequest()); }));
	mTouchMenu->setMenuItemModels(menuItemModels);

	mEngine.setTouchInfoPipeCallback([this, mTouchMenu](const ds::ui::TouchInfo& ti) { mTouchMenu->handleTouchInfo(ti); });

	// For this test app, we show the app to start with for simplicity
	// In a real scenario, you'll probably want to start idled / attracting
	mEngine.stopIdling();
}

void getting_started_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::getting_started_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
