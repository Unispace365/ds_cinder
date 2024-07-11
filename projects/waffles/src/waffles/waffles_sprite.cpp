#include "stdafx.h"

#include "waffles_sprite.h"

#include <Poco/File.h>
#include <Poco/Path.h>


#include <ds/content/content_events.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/media/player/web_player.h>
#include <ds/ui/menu/touch_menu.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/util/clip_plane.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/sprite/web.h>
#include <ds/util/string_util.h>


#include "app/app_defs.h"
//#include "app/helpers.h"
#include "waffles/waffles_events.h"
#include "waffles/touch_menu/touch_menu_graphics.h"
#include "waffles/viewers/viewer_controller.h"


namespace downstream {

template<class VC> WafflesSprite<VC>::WafflesSprite(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "waffles/waffles_sprite.xml") {
	static_assert(std::is_base_of_v<waffles::ViewerController, VC>, "VC must be a subclass of waffles::ViewerController");
	if (auto holdy = getSprite("viewer_controller_holdy")) {
		mViewerController = new VC(mEngine, ci::vec2(getSize()));
		holdy->addChildPtr(mViewerController);
	}

	enable(false);

	runLayout();

	setupTouchMenu();

	listenToEvents<waffles::ShowWaffles>([this](const auto& ev) { 
		onShow(ev);
	});

	listenToEvents<ds::app::IdleStartedEvent>([this](const auto& ev) {
		onIdleStarted(ev);
	});
	listenToEvents<waffles::HideWaffles>([this](const auto& ev) { 
		onHide(ev);	
	});
}

template <class VC>
void WafflesSprite<VC>::onIdleStarted(const ds::app::IdleStartedEvent& e) {
	mEngine.getNotifier().notify(waffles::HideWaffles());
}

template <class VC>
void WafflesSprite<VC>::setupTouchMenu() {
	mTouchMenu = nullptr;
	if (!mEngine.getAppSettings().getBool("five_finger_menu:enable", 0, true)) return;

	const float baseClusterRadius = mEngine.getAppSettings().getFloat("five_finger_menu:radius", 0, 300.0f);
	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage	 = "%APP%/data/images/ui/Touch_Menu_Blur_faded_600.png";
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mItemTitleOpacity	 = 0.9f;
	tmc.mClusterRadius		 = baseClusterRadius;
	tmc.mBackgroundOpacity	 = 0.95f;
	tmc.mBackgroundScale	 = 2.0f;
	tmc.mBackgroundColor	 = mEngine.getColors().getColorFromName("waffles:five_finger:circle");
	tmc.mBackgroundBlendMode = ds::ui::BlendMode::NORMAL;
	tmc.mItemSize			 = ci::vec2(mEngine.getAppSettings().getFloat("touch_menu:icon_size"));
	tmc.mItemIconHeight		 = mEngine.getAppSettings().getFloat("touch_menu:icon_height");
	tmc.mDoClipping			 = false;
	tmc.mAnimationDuration	 = mEngine.getAnimDur();
	tmc.mAnimationStyle		 = ds::ui::TouchMenu::TouchMenuConfig::kAnimateRadial;
	tmc.mActivatedCallback	 = [this, tmc](ds::ui::Sprite* mainSpr, ds::ui::Sprite* graphicSpr) {
		  if (!graphicSpr || !mainSpr) return;

		  bool graphicsExist = false;

		  for (auto it : graphicSpr->getChildren()) {
			  auto graphics = dynamic_cast<waffles::TouchMenuGraphics*>(it);
			  if (graphics) {
				  graphics->animateOn();
				  graphicsExist = true;
			  }
		  }
		  if (!graphicsExist) {
			  waffles::TouchMenuGraphics* gen = new waffles::TouchMenuGraphics(mEngine);
			  graphicSpr->addChildPtr(gen);
			  gen->animateOn();
		  }
	};

	tmc.mDeactivatedCallback = [](ds::ui::Sprite* mainSpr, ds::ui::Sprite* graphicSpr) {
		if (!graphicSpr || !mainSpr) return;

		for (auto it : graphicSpr->getChildren()) {
			auto graphics = dynamic_cast<waffles::TouchMenuGraphics*>(it);
			if (graphics) {
				graphics->animateOff();
			}
		}
	};

	ci::ColorA	 menuFg = mEngine.getColors().getColorFromName("waffles:five_finger:button:bg");
	ci::ColorA	 menuBg = mEngine.getColors().getColorFromName("waffles:five_finger:button:fg");
	std::wstring emptySubtitle;

	std::wstring overallTitle  = mEngine.getAppSettings().getWString("overall:title", 0, L"Waffles");
	bool		 allowCmsStuff = mEngine.getAppSettings().getBool("cms_files:allow", 0, true);

	auto closeModel = ds::ui::TouchMenu::MenuItemModel(
		L"Close Assets", "%APP%/data/images/waffles/icons/4x/Close_256.png", "%APP%/data/images/waffles/icons/4x/Close_Glow_256.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestCloseAllEvent(pos)); }, emptySubtitle, menuFg,
		menuBg);

	auto presNext = ds::ui::TouchMenu::MenuItemModel(
		L"Presentation Next", "%APP%/data/images/waffles/icons/4x/Forward_256.png",
		"%APP%/data/images/waffles/icons/4x/Forward_Glow_256.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestEngageNext()); }, emptySubtitle, menuFg, menuBg);

	auto arrange = ds::ui::TouchMenu::MenuItemModel(
		L"Arrange", "%APP%/data/images/waffles/icons/4x/Arrange_256.png", "%APP%/data/images/waffles/icons/4x/Arrange_Glow_256.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestArrangeEvent(pos)); }, emptySubtitle, menuFg,
		menuBg);

	auto homeView = ds::ui::TouchMenu::MenuItemModel(
		overallTitle, "%APP%/data/images/waffles/icons/4x/Home_256.png", "%APP%/data/images/waffles/icons/4x/Home_Glow_256.png",
		[this](ci::vec3 pos) {
			mEngine.getNotifier().notify(waffles::ShowWaffles());
			mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_LAUNCHER, pos, waffles::ViewerCreationArgs::kViewLayerTop)));
		},
		emptySubtitle, menuFg, menuBg);

	auto searchy = ds::ui::TouchMenu::MenuItemModel(
		L"Search", "%APP%/data/images/waffles/icons/4x/Search_256.png", "%APP%/data/images/waffles/icons/4x/Search_Glow_256.png",
		[this](ci::vec3 pos) {
			// mEngine.getNotifier().notify(waffles::ShowWaffles());
			mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_SEARCH, pos)));
		},
		emptySubtitle, menuFg, menuBg);

	auto gather = ds::ui::TouchMenu::MenuItemModel(
		L"Gather", "%APP%/data/images/waffles/icons/4x/Gather_256.png", "%APP%/data/images/waffles/icons/4x/Gather_Glow_256.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestGatherEvent(pos)); }, emptySubtitle, menuFg,
		menuBg);

	auto presBack = ds::ui::TouchMenu::MenuItemModel(
		L"Presentation Back", "%APP%/data/images/waffles/icons/4x/Backward_256.png",
		"%APP%/data/images/waffles/icons/4x/Backward_Glow_256.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestEngageBack()); }, emptySubtitle, menuFg, menuBg);

	auto presContr = ds::ui::TouchMenu::MenuItemModel(
		L"Presentation Control", "%APP%/data/images/waffles/icons/4x/Presentations_256.png",
		"%APP%/data/images/waffles/icons/4x/Presentations_Glow_256.png",
		[this](ci::vec3 pos) {
			// mEngine.getNotifier().notify(waffles::ShowWaffles());
			mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_PRESENTATION_CONTROLLER, pos,
									   waffles::ViewerCreationArgs::kViewLayerTop)));
		},
		emptySubtitle, menuFg, menuBg);

	auto states = ds::ui::TouchMenu::MenuItemModel(
		L"States", "%APP%/data/images/waffles/icons/4x/States_64.png", "%APP%/data/images/waffles/icons/4x/States_glow_64.png",
		[this](ci::vec3 pos) {
			mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_STATE_VIEWER, pos)));
		},
		emptySubtitle, menuFg, menuBg);

	auto exitApp = ds::ui::TouchMenu::MenuItemModel(
		L"Exit", "%APP%/data/images/waffles/icons/4x/Close_64.png", "%APP%/data/images/waffles/icons/4x/Close_Glow_64.png",
		[this](ci::vec3 pos) { mEngine.getNotifier().notify(waffles::RequestAppExit()); }, emptySubtitle, menuFg, menuBg);

	std::vector<ds::ui::TouchMenu::MenuItemModel> menuItemModels;

	std::string menuOrder = mEngine.getAppSettings().getString("five_finger_menu:order", 0, "");
	if (menuOrder.empty()) {
		menuItemModels.push_back(closeModel);

		if (allowCmsStuff) menuItemModels.push_back(searchy);
		if (allowCmsStuff) menuItemModels.push_back(presNext);

		menuItemModels.push_back(arrange);
		menuItemModels.push_back(homeView);
		menuItemModels.push_back(gather);

		if (allowCmsStuff) menuItemModels.push_back(presBack);
		if (allowCmsStuff) menuItemModels.push_back(presContr);

		menuItemModels.push_back(states);

		if (mEngine.getAppSettings().getBool("exit_app:allow", 0, false)) menuItemModels.push_back(exitApp);

	} else {
		auto theOrder = ds::split(menuOrder, ", ", true);
		for (auto it : theOrder) {
			if (it == "close") {
				menuItemModels.emplace_back(closeModel);
			} else if (it == "search" && allowCmsStuff) {
				menuItemModels.emplace_back(searchy);
			} else if (it == "pres_next" && allowCmsStuff) {
				menuItemModels.emplace_back(presNext);
			} /* else if (it == "arrange") {
				menuItemModels.emplace_back(arrange);
			} */
			else if (it == "home") {
				menuItemModels.emplace_back(homeView);
			} /* else if (it == "gather") {
				menuItemModels.emplace_back(gather);
			} */
			else if (it == "pres_back" && allowCmsStuff) {
				menuItemModels.emplace_back(presBack);
			} else if (it == "pres_cont" && allowCmsStuff) {
				menuItemModels.emplace_back(presContr);
			} else if (it == "states") {
				menuItemModels.emplace_back(states);
			} else if (it == "exit") {
				menuItemModels.emplace_back(exitApp);
			}
		}
	}

	mTouchMenu = new ds::ui::TouchMenu(mEngine);
	mTouchMenu->setMenuConfig(tmc);
	mTouchMenu->setMenuItemModels(menuItemModels);
	if (auto holdy = getSprite("five_finger_holdy")) {
		holdy->addChildPtr(mTouchMenu);
	} else {
		if (auto mainEngine = dynamic_cast<ds::Engine*>(&mEngine)) {
			mainEngine->getRootSprite().addChildPtr(mTouchMenu);
		}
	}
	// addChildPtr(mTouchMenu);

	mEngine.setTouchInfoPipeCallback([this](const ds::ui::TouchInfo& ti) {
		if (mTouchMenu) {
			mTouchMenu->handleTouchInfo(ti);
		}
	});
}

template <class VC>
void WafflesSprite<VC>::onSizeChanged() {
	clearChildren();

	if (auto holdy = getSprite("viewer_controller_holdy")) {
		mViewerController = new waffles::ViewerController(mEngine, ci::vec2(getSize()));
		holdy->addChildPtr(mViewerController);
	} else {
		mViewerController = nullptr;
	}
	runLayout();

	auto bounds = ci::Rectf(0.f, 0.f, getWidth(), getHeight());

	/* mViewerController->setLayerBounds(waffles::ViewerCreationArgs::kViewLayerBackground, bounds);
	mViewerController->setLayerBounds(waffles::ViewerCreationArgs::kViewLayerNormal, bounds); */
}

template <class VC>
void WafflesSprite<VC>::onShow(const waffles::ShowWaffles& e) {
	// Start presentation.
	userInputReceived();
	callAfterDelay(
		[this] {
			// auto alreadyPres = !mEngine.mContent.getChildByName("current_presentation").getChildren().empty();
			// if (!alreadyPres) {
			auto pres_id = downstream::getInitialPresentation(mEngine);
			if (!pres_id.empty()) {
				auto pres = downstream::getRecordByUid(mEngine, pres_id);
				if (pres.getChildren().size() > 0) { // activate first slide
					mEngine.getNotifier().notify(waffles::RequestEngagePresentation(pres.getChild(0), false));
					mEngine.mContent.setProperty("presentation_controller_blocked", false);
				}
			}
			// }
		},
		0.35f);
}

template <class VC>
void WafflesSprite<VC>::onHide(const waffles::HideWaffles& e) {}


} // namespace downstream
