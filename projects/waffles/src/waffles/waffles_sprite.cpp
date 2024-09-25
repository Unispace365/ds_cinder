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
#include <ds\app\engine\engine_standalone.h>

#include "app/waffles_app_defs.h"
//#include "app/helpers.h"
#include "waffles/waffles_events.h"
#include "waffles/touch_menu/touch_menu_graphics.h"
#include "waffles/viewers/viewer_controller.h"
#include "waffles/template/template_config.h"

#include "waffles/common/ui_utils.h"

#include "ds/content/platform.h"

namespace waffles {

WafflesSprite* WafflesSprite::mDefaultWaffles = nullptr;

WafflesSprite::WafflesSprite(ds::ui::SpriteEngine & eng)
: ds::ui::SmartLayout(eng, "waffles/waffles_sprite.xml"), mSettingsService(eng), mChannelClient() {

	

	auto sh = new waffles::ShadowLayout(eng);
	sh->release();
	auto pb = new waffles::PinboardButton(eng, "waffles/pinboard/pinboard_button.xml");
	pb->release();
	auto cap = new waffles::CapturePlayer(eng);
	cap->release();

	
}

WafflesSprite::~WafflesSprite() {
	if (mTimedCallback) {
		mEngine.cancelTimedCallback(mTimedCallback);
	}
	if (mDefaultWaffles == this) {
		mDefaultWaffles = nullptr;
	}
	if (mTemplateLayer && mTemplateLayer->getParent() == nullptr) {
		mTemplateLayer->release();
	}
	if (mBackgroundView && mBackgroundView->getParent() == nullptr) {
		mBackgroundView->release();
	}
	if (mViewerController && mViewerController->getParent() == nullptr) {
		mViewerController->release();
	}

}

void WafflesSprite::initializeWaffles(std::string eventChannel) {


	

	
	mChannelName = eventChannel;
	if (!eventChannel.empty()) {
		mChannelClient.setNotifier(mEngine.getChannel(eventChannel));
		mChannelClient.start();

	}
	else {
		mChannelClient.setNotifier(mEngine.getNotifier());
		mChannelClient.start();
	}


	//init the template config
	mTemplateConfig = TemplateConfig::getDefault();

	// Setup waffles events
	auto& reg = ds::event::Registry::get();
	
	reg.addEventCreator(waffles::RequestCloseAllEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestCloseAllEvent(); });
	reg.addEventCreator(waffles::RequestAppExit::NAME(), []() -> ds::Event* { return new waffles::RequestAppExit(); });
	reg.addEventCreator(waffles::RequestArrangeEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestArrangeEvent(ci::vec3()); });
	reg.addEventCreator(waffles::RequestGatherEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestGatherEvent(ci::vec3()); });
	reg.addEventCreator(waffles::RequestViewerLaunchEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestViewerLaunchEvent(waffles::ViewerCreationArgs()); });
	reg.addEventCreator(waffles::RequestPresentationAdvanceEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestPresentationAdvanceEvent(); });
	reg.addEventCreator(waffles::RequestPresentationStepRefresh::NAME(),
		[]() -> ds::Event* { return new waffles::RequestPresentationStepRefresh(); });
	reg.addEventCreator(waffles::RequestPresentationEndEvent::NAME(),
		[]() -> ds::Event* { return new waffles::RequestPresentationEndEvent(); });

	mViewerController = ViewerControllerFactory::getInstanceOf(ci::vec2(getSize()), eventChannel);


	if (auto holdy = getSprite("viewer_controller_holdy")) {

		holdy->addChildPtr(mViewerController);
	}
	auto backgroundMode = mEngine.getWafflesSettings().getString("layers:background:mode", 0, "createAndPlace");

	if (backgroundMode != "off") {
		mBackgroundView = new BackgroundView(mEngine);
		mBackgroundView->setSize(getWidth(), getHeight());
		auto holdy = getSprite("background_holdy");
		if (backgroundMode == "createAndPlace" && holdy ) {
			holdy->addChildPtr(mBackgroundView);
		}
	}

	auto templateMode = mEngine.getWafflesSettings().getString("layers:template:mode", 0, "createAndPlace");

	if (templateMode != "off") {
		mTemplateLayer = new TemplateLayer(mEngine, ci::vec2(getSize()), ci::vec2(0.0f, 0.0f));
		auto holdy = getSprite("template_holdy");
		if (templateMode == "createAndPlace" && holdy) {
			holdy->addChildPtr(mTemplateLayer);
		}
	}

	//** These are here to get initializers to run **//
	auto sh = new waffles::ShadowLayout(mEngine);
	sh->release();
	auto pb = new waffles::PinboardButton(mEngine, "waffles/pinboard/pinboard_button.xml");
	pb->release();
	auto cap = new waffles::CapturePlayer(mEngine);
	cap->release();
	//** End: These are here to get initializers to run **//

	enable(false);

	runLayout();

	setupTouchMenu();

	listenToEvents<waffles::ShowWaffles>([this](const auto& ev) { onShow(ev); });

	listenToEvents<ds::app::IdleStartedEvent>([this](const auto& ev) { 
		onIdleStarted(ev); 
	});
	listenToEvents<waffles::HideWaffles>([this](const auto& ev) { onHide(ev); });
	listenToEvents<ds::app::IdleEndedEvent>([this](const auto& ev) { 
		onIdleEnded(ev); 
	});
	listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) { onScheduleUpdated(ev); });
	listenToEvents<waffles::RequestPresentationEndEvent>([this](const auto& ev) { onPresentationEndRequest(ev); });
	listenToEvents<waffles::RequestEngagePresentation>([this](const auto& ev) { onPresentationStartRequest(ev); });
	listenToEvents<waffles::RequestEngageNext>([this](const auto& ev) { onNextRequest(ev); });
	listenToEvents<waffles::RequestEngageBack>([this](const auto& ev) { onBackRequest(ev); });
	listenToEvents<waffles::RequestPresentationAdvanceEvent>(
		[this](const auto& ev) { onPresentationAdvanceRequest(ev); });
	setDefaultPresentation();
	if (mDefaultWaffles == nullptr) {
		mDefaultWaffles = this;
	}

	//drawing uploads
	mDrawingUploadService = new DrawingUploadService(mEngine,eventChannel);

	mTimedCallback = mEngine.timedCallback(
		[this]() {
			auto helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
			auto model = ds::model::ContentModelRef("Empty");
			model.setProperty("type_uid", mTemplateConfig->getTemplateDefFromName("empty").id);
			ds::Resource r = helper->getBackgroundForPlatform();
			if (helper->getApplyParticles()) {
				mChannelClient.notify(waffles::RequestBackgroundChange(
					waffles::BACKGROUND_TYPE_PARTICLES, ds::model::ContentModelRef())); // 1 = BACKGROUND_TYPE_PARTICLES
			}
			else if (!r.empty()) {
				model.setPropertyResource("media_res", r);
				mChannelClient.notify(waffles::RequestBackgroundChange(waffles::BACKGROUND_TYPE_USER_MEDIA, model));
			}
			else {
				mChannelClient.notify(
					waffles::RequestBackgroundChange(waffles::BACKGROUND_TYPE_DEFAULT, ds::model::ContentModelRef()));
			}
		},
	0.1f);


}

//template <class VC>
void WafflesSprite::onIdleStarted(const ds::app::IdleStartedEvent& e) {
	//mEngine.getNotifier().notify(waffles::HideWaffles());
	//endPresentationMode();
	//do ambient
}

//template <class VC>
void WafflesSprite::onIdleEnded(const ds::app::IdleEndedEvent& e) {
	//setDefaultPresentation();
	//startPresentationMode();
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
	mChannelClient.notify(waffles::ChangeTemplateRequest(ds::model::ContentModelRef()));
	auto backer = getSprite("presentation_backer");
	if (backer && mEngine.getWafflesSettings().getBool("template:use_backer", 0, true)) {
		backer->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone(), [backer] { backer->hide(); });
	}
}

//template <class VC>
void WafflesSprite::onPresentationStartRequest(const waffles::RequestEngagePresentation& ev) {
	mAmEngaged = true;
	DS_LOG_INFO("got a request engage presentation event in engage controller");
	auto helper	 = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	auto content = ev.mContent;
	if (content.getPropertyString("type_key") == "pinboard_event") {
		if (mPlaylist.getPropertyString("type_key") != "pinboard_event") {

			cancelDelayedCall();
			auto playlist = ds::model::ContentModelRef("Pinboard");
			playlist.setProperty("type_key", mTemplateConfig->getTemplateDefFromName("pinboard_event").name);
			playlist.setProperty("record_name", content.getPropertyString("record_name"));
			auto model = ds::model::ContentModelRef("Pinboard");
			model.setProperty("type_uid", mTemplateConfig->getTemplateDefFromName("pinboard_event").id);
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
			playlist.setProperty("type_key", mTemplateConfig->getTemplateDefFromName("assets_mode").name);
			playlist.setProperty("record_name", std::string("Asset Mode"));
			auto model = ds::model::ContentModelRef("Assets");
			model.setProperty("type_uid", mTemplateConfig->getTemplateDefFromName("assets_mode").id);
			playlist.addChild(model);
			setPresentation(playlist);
			gotoItem(0);
		}
		return;
	}

	cancelDelayedCall();
	bool newPres  = false;
	bool newSlide = false;
	if (!ContentUtils::getDefault(mEngine)->isPresentation(content) &&
		!ContentUtils::getDefault(mEngine)->isAmbientPlaylist(content)) {
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

	auto backer = getSprite("presentation_backer");
	if (backer && mEngine.getWafflesSettings().getBool("template:use_backer",0,true)) {
		backer->show();
		backer->tweenOpacity(1.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone());
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

	mChannelClient.notify(waffles::PresentationStateChanged(mPlaylist));
	mChannelClient.notify(waffles::ChangeTemplateRequest(mSlide));

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
		[this](ci::vec3 pos) { mChannelClient.notify(waffles::RequestCloseAllEvent(pos));  }, emptySubtitle, menuFg,
		menuBg);

	auto presNext = ds::ui::TouchMenu::MenuItemModel(
		L"Presentation Next", "%APP%/data/images/waffles/icons/4x/Forward_256.png",
		"%APP%/data/images/waffles/icons/4x/Forward_Glow_256.png",
		[this](ci::vec3 pos) { mChannelClient.notify(waffles::RequestEngageNext()); }, emptySubtitle, menuFg, menuBg);

	auto arrange = ds::ui::TouchMenu::MenuItemModel(
		L"Arrange", "%APP%/data/images/waffles/icons/4x/Arrange_256.png", "%APP%/data/images/waffles/icons/4x/Arrange_Glow_256.png",
		[this](ci::vec3 pos) { mChannelClient.notify(waffles::RequestArrangeEvent(pos)); }, emptySubtitle, menuFg,
		menuBg);

	auto homeView = ds::ui::TouchMenu::MenuItemModel(
		overallTitle, "%APP%/data/images/waffles/icons/4x/Home_256.png", "%APP%/data/images/waffles/icons/4x/Home_Glow_256.png",
		[this](ci::vec3 pos) {
			mAmEngaged = true;
			mChannelClient.notify(waffles::ShowWaffles());
			mChannelClient.notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_LAUNCHER, pos, waffles::ViewerCreationArgs::kViewLayerTop)));
		},
		emptySubtitle, menuFg, menuBg);

	auto searchy = ds::ui::TouchMenu::MenuItemModel(
		L"Search", "%APP%/data/images/waffles/icons/4x/Search_256.png", "%APP%/data/images/waffles/icons/4x/Search_Glow_256.png",
		[this](ci::vec3 pos) {
			// mEngine.getNotifier().notify(waffles::ShowWaffles());
			mChannelClient.notify(waffles::RequestViewerLaunchEvent(
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
			mChannelClient.notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_PRESENTATION_CONTROLLER, pos,
									   waffles::ViewerCreationArgs::kViewLayerTop)));
		},
		emptySubtitle, menuFg, menuBg);

	auto states = ds::ui::TouchMenu::MenuItemModel(
		L"States", "%APP%/data/images/waffles/icons/4x/States_64.png", "%APP%/data/images/waffles/icons/4x/States_glow_64.png",
		[this](ci::vec3 pos) {
			mChannelClient.notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_STATE_VIEWER, pos)));
		},
		emptySubtitle, menuFg, menuBg);

	auto exitApp = ds::ui::TouchMenu::MenuItemModel(
		L"Exit", "%APP%/data/images/waffles/icons/4x/Close_64.png", "%APP%/data/images/waffles/icons/4x/Close_Glow_64.png",
		[this](ci::vec3 pos) { mChannelClient.notify(waffles::RequestAppExit()); }, emptySubtitle, menuFg, menuBg);

	auto blank = ds::ui::TouchMenu::MenuItemModel(
		L"", "", "",
		[this](ci::vec3 pos) {/*do nothing*/ }, emptySubtitle, menuFg, menuBg);



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
			} else if (it == "arrange") {
				menuItemModels.emplace_back(arrange);
			} 
			else if (it == "home") {
				menuItemModels.emplace_back(homeView);
			} else if (it == "gather") {
				menuItemModels.emplace_back(gather);
			}
			else if (it == "pres_back" && allowCmsStuff) {
				menuItemModels.emplace_back(presBack);
			} else if (it == "pres_cont" && allowCmsStuff) {
				menuItemModels.emplace_back(presContr);
			} else if (it == "states") {
				menuItemModels.emplace_back(states);
			} else if (it == "exit") {
				menuItemModels.emplace_back(exitApp);
			}
			else if (it == "blank") {
				menuItemModels.emplace_back(blank);
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
		}
	}
	// addChildPtr(mTouchMenu);

	mEngine.setTouchInfoPipeCallback([this](const ds::ui::TouchInfo& ti) {
		if (ti.mPhase == ds::ui::TouchInfo::Removed) {
			mTouchPoints[ti.mFingerId] = ci::vec2(-1.0f, -1.0f);
		} else {
			mTouchPoints[ti.mFingerId] = ci::vec2(ti.mCurrentGlobalPoint);
		}

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
	mChannelClient.notify(waffles::EngageStarted());

	if (!mPlaylistUid.empty()) {
		// Always start on the first playlist item when starting the engage
		cancelDelayedCall();


		// Wait for background to finish transitioning to empty, then trigger the template
		mChannelClient.listenToEvents<waffles::BackgroundChangeComplete>([this](const auto& ev) {
			if (mAmEngaged) {
				// Only trigger the template if we're still in engage mode
				gotoItem(mPlaylistIdx);
			}

			// Stop listening
			mChannelClient.stopListeningToEvents<waffles::BackgroundChangeComplete>();
		});
	} else {
		mChannelClient.notify(waffles::ChangeTemplateRequest());

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

	auto currPres = mEngine.mContent.getChildByName("current_presentation");
	currPres.clearChildren();
	mEngine.mContent.replaceChild(currPres);

	// Revert to no template
	mChannelClient.notify(waffles::RequestCloseAllEvent(true));
	mChannelClient.notify(waffles::ChangeTemplateRequest());

	

	// Notify listeners.
	mChannelClient.notify(waffles::EngageEnded());
	auto backer = getSprite("presentation_backer");
	if (backer && mEngine.getWafflesSettings().getBool("template:use_backer", 0, true)) {
		backer->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone(), [backer] { backer->hide(); });
	}
}

//template <class VC>
void WafflesSprite::setData() {
	ds::model::ContentModelRef thePlaylist;
	auto					   helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	if (!mPlaylistUid.empty()) {
		thePlaylist = helper->getRecordByUid(mPlaylistUid);
	} else {
		thePlaylist = ds::model::ContentModelRef("Empty Playlist");
		thePlaylist.setProperty("type_key", mTemplateConfig->getTemplateDefFromName("empty").name);
		auto model = ds::model::ContentModelRef("Empty");
		model.setProperty("type_uid", mTemplateConfig->getTemplateDefFromName("empty").id);
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



// template <class VC>
void WafflesSprite::onSizeChanged() {
	

	if (auto holdy = getSprite("viewer_controller_holdy")) {
		holdy->clearChildren();
		auto size = getSize();
		mViewerController = ViewerControllerFactory::getInstanceOf(ci::vec2(size),mChannelName);
		holdy->addChildPtr(mViewerController);
	} else {
		mViewerController = nullptr;
	}
	//if (mViewerController) {
	//	mViewerController->setSize(ci::vec2(getSize()));
	//}
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
					mChannelClient.notify(waffles::RequestEngagePresentation(pres.getChild(0), false));
					//mEngine.mContent.setProperty("presentation_controller_blocked", false);
				}
			}
			// }
		},
		0.35f);
}

//template <class VC>
void WafflesSprite::onHide(const waffles::HideWaffles& e) {}




} // namespace waffles
