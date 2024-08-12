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
#include "waffles/template/template_config.h"
#include "ds/content/platform.h"

namespace waffles {

//template <class VC>
void WafflesSprite::onIdleStarted(const ds::app::IdleStartedEvent& e) {
	mEngine.getNotifier().notify(waffles::HideWaffles());
	endPresentationMode();
}

//template <class VC>
void WafflesSprite::onIdleEnded(const ds::app::IdleEndedEvent& e) {
	setDefaultPresentation();
	startPresentationMode();
}

//template <class VC>
void WafflesSprite::onScheduleUpdated(const ds::ScheduleUpdatedEvent& e) {
	if (mEngine.getWafflesSettings().getBool("app:editor_mode", 0, false)) {
		// In "editor mode", refresh the current slide whenever it changes
		auto currPres		   = mEngine.mContent.getChildByName("current_presentation");
		auto currSlide		   = currPres.getPropertyInt("current_slide");
		auto currentSlideModel = currPres.getChild(0).getChild(currSlide - 1);
		if (!currPres.empty() && !currentSlideModel.empty()) {
			setData();
			if (currentSlideModel != mPlaylist.getChild(currSlide - 1)) {
				gotoItem(currSlide - 1);
			}
		} else {
			setData();
		}
	} else {
		// Otherwise get the new data but don't refresh :)
		// New content will be visisble when leaving and returning to the slide
		setData();
	}
}

//template <class VC>
void WafflesSprite::onPresentationEndRequest(const waffles::RequestPresentationEndEvent& e) {
	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.clearChildren();
	mEngine.mContent.replaceChild(currPres);
}

//template <class VC>
void WafflesSprite::onPresentationStartRequest(const waffles::RequestEngagePresentation& ev) {
	DS_LOG_INFO("got a request engage presentation event in engage controller");
	auto helper	 = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	auto content = ev.mContent;
	if (content.getPropertyString("type_key") == "pinboard_event") {
		if (mPlaylist.getPropertyString("type_key") != "pinboard_event") {

			cancelDelayedCall();
			auto playlist = ds::model::ContentModelRef("Pinboard");
			playlist.setProperty("type_key", waffles::getTemplateDefFromName("pinboard_event").name);
			playlist.setProperty("record_name", content.getPropertyString("record_name"));
			auto model = ds::model::ContentModelRef("Pinboard");
			model.setProperty("type_uid", waffles::getTemplateDefFromName("pinboard_event").id);
			model.setProperty("record_name", content.getPropertyString("record_name"));
			playlist.addChild(model);
			setPresentation(playlist);
			gotoItem(0);
		}
		return;

	} else if (content.getName() == "assets") {
		if (mPlaylist.getPropertyString("type_key") != "assets_mode") {
			cancelDelayedCall();
			auto playlist = ds::model::ContentModelRef("Assets");
			playlist.setProperty("type_key", waffles::getTemplateDefFromName("assets_mode").name);
			playlist.setProperty("record_name", std::string("Asset Mode"));
			auto model = ds::model::ContentModelRef("Assets");
			model.setProperty("type_uid", waffles::getTemplateDefFromName("assets_mode").id);
			playlist.addChild(model);
			setPresentation(playlist);
			gotoItem(0);
		}
		return;
	}

	cancelDelayedCall();
	bool newPres  = false;
	bool newSlide = false;
	if (content.getPropertyString("type_key") != "interactive_playlist" &&
		content.getPropertyString("type_key") != "ambient_playlist") {
		// This is a slide
		// Might be a new pres, might not be

		auto theSlide = content;
		content		  = helper->getRecordByUid(theSlide.getPropertyString("parent_uid"));
		auto uid	  = content.getPropertyString("uid");
		if (uid != mPlaylistUid) {
			// New pres
			mPlaylistUid = uid;
			newPres		 = true;
		}
		for (auto slide : content.getChildren()) {
			if (slide.getPropertyString("uid") == theSlide.getPropertyString("uid")) {
				auto slideUid = slide.getPropertyString("uid");
				if (slideUid != mSlideUid) {
					mSlideUid = slideUid;
					newSlide  = true;
				}
				break;
			}
		}
	} else {
		// This is a presentation, might be new, might be same
		auto uid = content.getPropertyString("uid");
		if (uid != mPlaylistUid) {
			mPlaylistUid = uid;
			newPres		 = true;
		}
	}

	if (newPres) {
		setPresentation(helper->getRecordByUid(mPlaylistUid));
	}

	if (newSlide) {
		setSlide(helper->getRecordByUid(mSlideUid));
	} else {
		setData();
		if (!mPlaylist.getChildren().empty()) setSlide(mPlaylist.getChildren().front());
	}

	if (!mEngine.isIdling() && mAmEngaged) {
		gotoItem(mPlaylistIdx);
	}
}

//template <class VC>
void WafflesSprite::onNextRequest(const waffles::RequestEngageNext& e) {
	if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
		mPlaylist.getPropertyString("type_key") == "assets_mode") {
		return;
	}

	nextItem();
}

//template <class VC>
void WafflesSprite::onBackRequest(const waffles::RequestEngageBack& e) {
	if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
		mPlaylist.getPropertyString("type_key") == "assets_mode") {
		return;
	}
	prevItem();
}

//template <class VC>
void WafflesSprite::onPresentationAdvanceRequest(const waffles::RequestPresentationAdvanceEvent& ev) {
	bool isFwd = (!ev.mUserStringData.empty() && ev.mUserStringData == "false") ? false : true;

	if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
		mPlaylist.getPropertyString("type_key") == "assets_mode") {
		return;
	}

	// setData();
	if (isFwd) {
		nextItem();
	} else {
		prevItem();
	}
}



//template <class VC>
void WafflesSprite::gotoItem(int index) {
	int childCount = mPlaylist.getChildren().size();
	if (childCount == 0) return;

	if (index >= childCount) {
		index = 0;
	} else if (index < 0) {
		index = childCount - 1;
	}

	mPlaylistIdx  = index;
	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.setName("current_presentation");
	currPres.setChildren({mPlaylist});
	currPres.setProperty("current_slide", mPlaylistIdx + 1);
	mEngine.mContent.replaceChild(currPres);

	mSlide	  = mPlaylist.getChildren()[mPlaylistIdx];
	mSlideUid = mSlide.getPropertyString("uid");

	// mEngine.getNotifier().notify(waffles::RequestCloseAllEvent(true));

	if (mPlaylist.getPropertyString("type_key") == "pinboard_event" ||
		mPlaylist.getPropertyString("type_key") == "assets_mode") {
		mPlaylistUid.clear();
	}

	mEngine.getNotifier().notify(waffles::PresentationStateChanged(mPlaylist));
	mEngine.getNotifier().notify(waffles::ChangeTemplateRequest(mSlide));

	if (mPlaylist.getPropertyString("type_key") == "empty") {
		callAfterDelay(
			[this] {
				setDefaultPresentation();
				setData();
			},
			0.25f);
	}
}

//template <class VC>
void WafflesSprite::setupTouchMenu() {
	mTouchMenu = nullptr;
	if (!mEngine.getWafflesSettings().getBool("five_finger_menu:enable", 0, true)) return;
	auto scale = mEngine.getWafflesSettings().getFloat("waffles:sprite:scale", 0, 1.0f);
	const float baseClusterRadius = mEngine.getWafflesSettings().getFloat("five_finger_menu:radius", 0, 300.0f);
	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage = mEngine.getWafflesSettings().getString("five_finger_menu:background:image", 0, "%APP%/data/images/waffles/ui/Touch_Menu_Blur_faded_600.png");
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mItemTitleOpacity	 = 0.9f;
	tmc.mClusterRadius		 = baseClusterRadius;
	tmc.mBackgroundOpacity	 = 0.95f;
	tmc.mBackgroundScale	 = mEngine.getWafflesSettings().getFloat("five_finger_menu:background:scale", 0, 2.0f*scale);
	tmc.mBackgroundColor	 = mEngine.getColors().getColorFromName("waffles:five_finger:circle");
	tmc.mBackgroundBlendMode = ds::ui::BlendMode::NORMAL;
	tmc.mItemSize			 = ci::vec2(mEngine.getWafflesSettings().getFloat("touch_menu:icon_size"));
	tmc.mItemIconHeight		 = mEngine.getWafflesSettings().getFloat("touch_menu:icon_height");
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

	std::wstring overallTitle  = mEngine.getWafflesSettings().getWString("overall:title", 0, L"Waffles");
	bool		 allowCmsStuff = mEngine.getWafflesSettings().getBool("cms_files:allow", 0, true);

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

	std::string menuOrder = mEngine.getWafflesSettings().getString("five_finger_menu:order", 0, "");
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

		if (mEngine.getWafflesSettings().getBool("exit_app:allow", 0, false)) menuItemModels.push_back(exitApp);

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

//template <class VC>
void WafflesSprite::setDefaultPresentation() {
	auto				helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	ds::model::Platform platformObj(mEngine);
	if (platformObj.getPlatformType() == ds::model::Platform::UNDEFINED) return;
	auto thePlaylist = helper->getPresentation();
	if (!thePlaylist.empty() && !thePlaylist.getChildren().empty()) {
		mPlaylist	  = thePlaylist;
		mPlaylistUid  = thePlaylist.getPropertyString("uid");
		auto theSlide = thePlaylist.getChildren().front();
		mSlideUid	  = theSlide.getPropertyString("uid");
	} else {
		mPlaylistUid.clear();
	}
}

void WafflesSprite::setPresentation(ds::model::ContentModelRef thePlaylist) {
	mPlaylist = thePlaylist;
}

bool WafflesSprite::setSlide(ds::model::ContentModelRef theSlide) {
	int idx = 0;
	for (auto slide : mPlaylist.getChildren()) {
		if (slide.getPropertyString("uid") == mSlideUid) {
			if (slide == mSlide) {
				// Already here, no need to change
				mPlaylistIdx = idx;
				return false;
			} else {
				mPlaylistIdx = idx;
				return true;
			}
			// break;
		}
		idx++;
	}

	return false;
}

//template <class VC>
void WafflesSprite::startPresentationMode() {
	mAmEngaged	 = true;
	mPlaylistIdx = 0;

	// Notify listeners.
	mEngine.getNotifier().notify(waffles::EngageStarted());

	if (!mPlaylistUid.empty()) {
		// Always start on the first playlist item when starting the engage
		cancelDelayedCall();


		// Wait for background to finish transitioning to empty, then trigger the template
		mEventClient.listenToEvents<waffles::BackgroundChangeComplete>([this](const auto& ev) {
			if (mAmEngaged) {
				// Only trigger the template if we're still in engage mode
				gotoItem(mPlaylistIdx);
			}

			// Stop listening
			mEventClient.stopListeningToEvents<waffles::BackgroundChangeComplete>();
		});
	} else {
		mEngine.getNotifier().notify(waffles::ChangeTemplateRequest());

		callAfterDelay(
			[this] {
				setDefaultPresentation();
				setData();
				startPresentationMode();
			},
			0.25f);
	}
}

//template <class VC>
void WafflesSprite::endPresentationMode() {
	cancelDelayedCall();

	mAmEngaged	 = false;
	mPlaylistIdx = -1;
	// Revert to no template
	mEngine.getNotifier().notify(waffles::RequestCloseAllEvent(true));
	mEngine.getNotifier().notify(waffles::ChangeTemplateRequest());

	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.clearChildren();
	mEngine.mContent.replaceChild(currPres);

	// Notify listeners.
	mEngine.getNotifier().notify(waffles::EngageEnded());
}

//template <class VC>
void WafflesSprite::setData() {
	ds::model::ContentModelRef thePlaylist;
	auto					   helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	if (!mPlaylistUid.empty()) {
		thePlaylist = helper->getRecordByUid(mPlaylistUid);
	} else {
		thePlaylist = ds::model::ContentModelRef("Empty Playlist");
		thePlaylist.setProperty("type_key", waffles::getTemplateDefFromName("empty").name);
		auto model = ds::model::ContentModelRef("Empty");
		model.setProperty("type_uid", waffles::getTemplateDefFromName("empty").id);
		thePlaylist.addChild(model);
		mPlaylistUid.clear();
	}

	if (thePlaylist != mPlaylist) {
		mPlaylist = thePlaylist;
	}
}


void WafflesSprite::nextItem() {
	gotoItem(mPlaylistIdx + 1);
}

void WafflesSprite::prevItem() {
	gotoItem(mPlaylistIdx - 1);
}

//template <class VC>
void WafflesSprite::onSizeChanged() {
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

//template <class VC>
void WafflesSprite::onShow(const waffles::ShowWaffles& e) {
	// Start presentation.
	userInputReceived();
	callAfterDelay(
		[this] {
			auto helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
			// auto alreadyPres = !mEngine.mContent.getChildByName("current_presentation").getChildren().empty();
			// if (!alreadyPres) {
			auto pres_id = helper->getInitialPresentationUid();
			if (!pres_id.empty()) {
				auto pres = helper->getRecordByUid(pres_id);
				if (pres.getChildren().size() > 0) { // activate first slide
					mEngine.getNotifier().notify(waffles::RequestEngagePresentation(pres.getChild(0), false));
					mEngine.mContent.setProperty("presentation_controller_blocked", false);
				}
			}
			// }
		},
		0.35f);
}

//template <class VC>
void WafflesSprite::onHide(const waffles::HideWaffles& e) {}


} // namespace downstream
