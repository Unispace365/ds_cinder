#include "stdafx.h"

#include "interface_layer.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/menu/touch_menu.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
//#include "app/helpers.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/touch_menu/touch_menu_graphics.h"
#include <ds/ui/button/layout_button.h>
// #include "waffles/viewers/viewer_controller.h"



namespace waffles {

InterfaceLayer::InterfaceLayer(ds::ui::SpriteEngine& eng, bool isReceiver)
	: ds::ui::SmartLayout(eng, "waffles/layer/interface_layer.xml")
	, mPulseTimer(mEngine) {

	for (auto [name, idx] : std::vector<std::pair<std::string, int>>{
			 {"left_menu.the_btn",  0},
			   {"right_menu.the_btn", 1}
	}) {
		if (auto menuBtn = getSprite<ds::ui::LayoutButton>(name)) {
			menuBtn->setClickFn([this, menuBtn, idx = idx] {
				auto pos = menuBtn->getGlobalPosition(); // + ci::vec3(60.f, 150.f, 0.f);
				pos += mEngine.getAppSettings().getVec3("persistent_menu:offset:" + ds::value_to_string(idx), 0,
														ci::vec3(0.f));
				/* pos.x -= 180.f;
				pos.y += 200.f; */
				mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
					waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_LAUNCHER_PERSISTANT, pos,
										   waffles::ViewerCreationArgs::kViewLayerNormal, 0.f, true, false, false)));
			});
		}
	}

	listenToEvents<ds::app::IdleStartedEvent>([this](const auto& ev) {
		mAllDialsInactive = true;
		tweenAnimateOff(true, 0.f, 0.0f);
	});
	listenToEvents<ds::app::IdleEndedEvent>([this](const auto& ev) { tweenAnimateOn(true, 0.f, 0.0f); });

	if (!isReceiver) {
		// Setup the touch menu
		setupTouchMenu();

		if (mEngine.getAppSettings().getString("app:mode", 0, "single") == "multi" &&
			mEngine.getAppSettings().getBool("app:fake_dials", 0, false)) {
			// Create a fake dial for each new presentation visited
			listenToEvents<waffles::PresentationStateChanged>([this](const waffles::PresentationStateChanged& ev) {
				auto requestedItem = ev.mContent;
				auto requestedType = requestedItem.getPropertyString("type_key");
				if (requestedType == "pinboard_event" || requestedType == "assets_mode") {
					return;
				}

				bool hasDial	   = false;
				bool isActive	   = false;
				int	 foundFakeCode = 0;
				int	 foundCode	   = 0;
				for (auto obj : mLiveObjects) {
					// If pres matches, or if "pres" is a child of a presentation assigned to a dial, we don't need a
					// fake dial
					auto codiceObj = getRecordForCodice(obj);
					if (codiceObj.empty()) continue;
					auto targetPresUid = codiceObj.getPropertyString("target_presentation");
					if (targetPresUid.empty()) {
						// Assets object
						/* hasDial = true;
						isActive  = (obj == mLiveObjects.back());
						foundCode = obj;
						break;*/
					} else if (requestedItem.getPropertyString("uid") == targetPresUid ||
							   requestedItem.getPropertyString("parent_uid") == targetPresUid) {
						//  Presentation object
						hasDial	 = true;
						isActive = (obj == mLiveObjects.back());
						// foundCode = obj;
						if (obj > 0) {
							// break;
							foundCode = obj;
						} else {
							foundFakeCode = obj;
							if (foundCode <= 0) {
								foundCode = obj;
							}
						}
					}
				}

				if (!hasDial) {
					auto allFakes = mEngine.mContent.getChildByName("fake_codice");
					allFakes.setName("fake_codice");
					allFakes.setProperty("record_name", std::string("Fake Codice Codes"));
					auto fakeCode = ds::model::ContentModelRef(ds::value_to_string(mNextFakeCode));
					fakeCode.setProperty("record_name", ds::value_to_string(mNextFakeCode));
					fakeCode.setProperty("type_key", std::string("codice_object"));
					fakeCode.setProperty("codice_code", mNextFakeCode);

					if (requestedType == "interactive_playlist" || requestedType == "ambient_playlist") {
						fakeCode.setProperty("target_presentation", requestedItem.getPropertyString("uid"));
					} else if (requestedType != "pinboard_event" && requestedType != "assets" &&
							   requestedType != "folder" && requestedType != "codice_object") {
						fakeCode.setProperty("target_presentation", requestedItem.getPropertyString("parent_uid"));
					} else {
						return;
					}
					allFakes.addChild(fakeCode);
					mEngine.mContent.replaceChild(allFakes);

					auto lastPos = mEngine.mContent.getChildByName("last_touch").getPropertyVec2("pos") +
								   mEngine.getAppSettings().getVec2("dial:fake_offset", 0, ci::vec2(100.f, 100.0));
					auto finalPos = (ci::vec3(lastPos, 0.f));

					auto obj = ds::TuioObject(mNextFakeCode, finalPos, 0.f);

					mLiveObjects.push_back(obj.getObjectId());

					auto model = ds::model::ContentModelRef("dial");
					model.setProperty("id", obj.getObjectId());
					model.setProperty("pos", finalPos);
					mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(waffles::ViewerCreationArgs(
						model, waffles::VIEW_TYPE_CUSTOM_MENU, finalPos, waffles::ViewerCreationArgs::kViewLayerNormal)));

					mNextFakeCode -= 1;
				} else if (!isActive || mAllDialsInactive || (foundFakeCode < 0 && foundFakeCode != foundCode)) {
					if (foundFakeCode < 0 && foundFakeCode != foundCode) {
						mEngine.getNotifier().notify(waffles::RequestTuioObjectRelease(foundFakeCode));
					}
					auto findy = std::find(mLiveObjects.begin(), mLiveObjects.end(), foundCode);
					if (findy == mLiveObjects.end()) return;
					mLiveObjects.erase(findy);
					mLiveObjects.push_back(foundCode);
					mEngine.getNotifier().notify(waffles::RequestTuioObjectTrigger(foundCode));
				}
			});
		}

		listenToEvents<waffles::TuioObjectReleased>([this](const waffles::TuioObjectReleased& ev) {
			DS_LOG_INFO("Got obj released " << ev.mId);
			auto findy = std::find(mLiveObjects.begin(), mLiveObjects.end(), ev.mId);
			if (findy == mLiveObjects.end()) return;
			mLiveObjects.erase(findy);

			if (!mEngine.isIdling()) {
				if (!mLiveObjects.empty()) {
					DS_LOG_INFO("Should trigger! ID:" << mLiveObjects.back());
					mEngine.getNotifier().notify(waffles::RequestTuioObjectTrigger(mLiveObjects.back()));
				} else {
					mEngine.startIdling();
				}
			}
		});

		listenToEvents<waffles::TuioObjectTriggered>([this](const waffles::TuioObjectTriggered& ev) {
			DS_LOG_INFO("Got obj (re)triggered " << ev.mId);
			if (ev.mId == 0) {
				mAllDialsInactive = true;
				return;
			}
			auto findy = std::find(mLiveObjects.begin(), mLiveObjects.end(), ev.mId);
			if (findy == mLiveObjects.end()) {
				return;
			}
			mLiveObjects.erase(findy);
			mLiveObjects.push_back(ev.mId);
			mAllDialsInactive = false;
		});

		listenToEvents<waffles::TuioObjectEvent>([this](const waffles::TuioObjectEvent& ev) {
			if (std::find(mLiveObjects.begin(), mLiveObjects.end(), ev.mObj.getObjectId()) != mLiveObjects.end())
				return;


			switch (ev.mPhase) {
			case ds::ui::TouchInfo::Added: {
				mLiveObjects.push_back(ev.mObj.getObjectId());

				auto pos = globalToLocal(ci::vec3(ev.mObj.getPosition(), 0.f));

				auto model = ds::model::ContentModelRef("dial");
				model.setProperty("id", ev.mObj.getObjectId());
				model.setProperty("pos", pos);
				mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(waffles::ViewerCreationArgs(
					model, waffles::VIEW_TYPE_CUSTOM_MENU, pos, waffles::ViewerCreationArgs::kViewLayerNormal)));

			} break;
			// Only care about added here
			case ds::ui::TouchInfo::Moved:
			case ds::ui::TouchInfo::Removed:
				break;
			}
		});
	} else {
		if (auto persistentMenu = getSprite("persistent_menu")) {
			persistentMenu->hide();
		}
	}
}

ds::model::ContentModelRef InterfaceLayer::getRecordForCodice(int id) {
	// Check real codices first
	/* for (auto child : mEngine.mContent.getChildByName("all_records").getChildren()) {
		if (child.getPropertyString("type_key") == "codice_object" && child.getPropertyInt("codice_code") == id) {
			mObjectMap[id] = child;
			return child;
			break;
		}
	}*/

	// Then check known live objects
	auto findy = mObjectMap.find(id);
	if (findy != std::end(mObjectMap)) {
		return findy->second;
	}

	// Finally check fakes
	{
		// Try all the real codices
		for (auto child : mEngine.mContent.getChildByName("all_records").getChildren()) {
			if (child.getPropertyString("type_key") == "codice_object" && child.getPropertyInt("codice_code") == id) {
				mObjectMap[id] = child;
				return child;
				break;
			}
		}

		for (auto child : mEngine.mContent.getChildByName("fake_codice").getChildren()) {
			if (child.getPropertyString("type_key") == "codice_object" && child.getPropertyInt("codice_code") == id) {
				mObjectMap[id] = child;
				return child;
				break;
			}
		}
	}

	// Return empty ref on failure
	return {};
}

void InterfaceLayer::setupTouchMenu() {
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
	tmc.mBackgroundColor	 = mEngine.getColors().getColorFromName("waffles:five_finger:circle");;
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

	ci::ColorA	 menuFg = mEngine.getColors().getColorFromName("waffles:five_finger:button:fg");
	ci::ColorA	 menuBg = mEngine.getColors().getColorFromName("waffles:five_finger:button:bg");
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
			mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(ds::model::ContentModelRef(), waffles::VIEW_TYPE_LAUNCHER, pos)));
		},
		emptySubtitle, menuFg, menuBg);

	auto searchy = ds::ui::TouchMenu::MenuItemModel(
		L"Search", "%APP%/data/images/waffles/icons/4x/Search_256.png", "%APP%/data/images/waffles/icons/4x/Search_Glow_256.png",
		[this](ci::vec3 pos) {
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
			} else if (it == "arrange") {
				menuItemModels.emplace_back(arrange);
			} else if (it == "home") {
				menuItemModels.emplace_back(homeView);
			} else if (it == "gather") {
				menuItemModels.emplace_back(gather);
			} else if (it == "pres_back" && allowCmsStuff) {
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

	mEngine.setTouchInfoPipeCallback(
		[this, previousCB = mEngine.getTouchInfoPipeCallback()](const ds::ui::TouchInfo& ti) {
			if (mTouchMenu) {
				mTouchMenu->handleTouchInfo(ti);
			}

			if (previousCB) previousCB(ti);
		});
}

} // namespace waffles
