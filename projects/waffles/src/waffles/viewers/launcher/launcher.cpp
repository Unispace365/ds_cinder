#include "stdafx.h"

#include "launcher.h"

#include <fstream>
#include <iostream>

#include <ds/app/engine/engine_roots.h>
#include <ds/app/environment.h>
#include <ds/content/content_events.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"
//#include "app/helpers.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/query/search_query.h"
#include "waffles/common/ui_utils.h"

//using namespace downstream;

namespace waffles {

Launcher::Launcher(ds::ui::SpriteEngine& g, bool hideClose)
	: BaseElement(g)
	, mEventClient(g) {

	

	mEngine.getNotifier().notify(waffles::WafflesLauncherOpened());

	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_LAUNCHER;
	mCanResize			  = true;
	mCanArrange			  = false;

	mPrimaryLayout = new ds::ui::SmartLayout(mEngine, "waffles/launcher/launcher.xml");
	mWafflesScale  = mEngine.getWafflesSettings().getFloat("waffles:sprite:scale", 0, 1.f);
	addChildPtr(mPrimaryLayout);

	if (auto closeBtn = mPrimaryLayout->getSprite("close_button.the_button")) {
		if (hideClose) {
			closeBtn->hide();
			closeBtn->enable(false);
			// Disable dragging?
			/* setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) {});
			enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
			enable(false); */
		} else {
			mPrimaryLayout->setSpriteClickFn("close_button.the_button", [this] {
				if (mCloseRequestCallback) mCloseRequestCallback();
			});
		}
	}

	// Refresh the menu items when the content changes
	mEventClient.listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) {
		callAfterDelay(
			[this] {
				updateMenuItems();
				setupMenuItems();
				handleSelection();
			},
			0.01f);
	});
	mEventClient.listenToEvents<RequestEngagePresentation>([this](const auto& ev) {
		bool hide_launcher = ev.mHideLauncher;
		callAfterDelay(
			[this, hide_launcher] {
				updateMenuItems();
				setupMenuItems();
				handleSelection();
				if (hide_launcher) {
					mCloseRequestCallback();
				}
			},
			0.01f);
	});
	mEventClient.listenToEvents<RequestEngageBack>(
		[this](const auto& ev) { callAfterDelay([this] { handleSelection(); }, 0.01f); });

	mEventClient.listenToEvents<RequestEngageNext>(
		[this](const auto& ev) { callAfterDelay([this] { handleSelection(); }, 0.01f); });

	mEventClient.listenToEvents<waffles::RequestPresentationAdvanceEvent>(
		[this](const auto& ev) { callAfterDelay([this] { handleSelection(); }, 0.01f); });

	mEventClient.listenToEvents<TemplateChangeComplete>([this](const auto& ev) { activatePanel(); });

	mEventClient.listenToEvents<waffles::WafflesFilterEvent>([this](const waffles::WafflesFilterEvent& ev) {
		auto helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
		if (ev.mType == mFilterSelected) return;
		mFilterSelected = ev.mType;
		DS_LOG_INFO("Waffles filtering by '" << mFilterSelected << "'.");
		mFolderStack.clear();
		if (auto back_button = mPrimaryLayout->getSprite("back_button")) {
			back_button->hide();
		}
		if (ev.mFromButton) {
			if (mSecondCloseButton) {
				mPrimaryLayout->getSprite("close_button.the_button")->mLayoutFudge =
					mEngine.getWafflesSettings().getVec3("launcher:close:normal:offset", 0, ci::vec3(0, 0, 0));
			}
		}
		std::vector<ds::model::ContentModelRef> allContent;

		auto pinny = helper->getPinboard();
		if (!pinny.empty()) allContent.push_back(pinny);

		auto allValid = mEngine.mContent.getKeyReferences(ds::model::VALID_MAP);

		
		for (auto& [key, value] : allValid) {
			allContent.push_back(value);
		}
		//allContent.insert(allContent.end(), allValid.begin(), allValid.end());

		if (mFilterSelected != "Folders") {
			allContent = recurseContent(allContent);
		}

		auto panel_content = ds::model::ContentModelRef(mFilterSelected);
		for (auto content : allContent) {
			if (filterValid(mFilterSelected, content) && unrepeatedContent(panel_content, content)) {
				panel_content.addChild(content);
			}
		}

		// recent files need to be sorted correctly, not by all content ordering :(
		if (mFilterSelected == "Recent") {
			auto unorderedPanelContent = panel_content.getChildren();
			panel_content.clearChildren();
			for (auto uid : mRecentFilterUids) {
				auto found = std::find_if(unorderedPanelContent.begin(), unorderedPanelContent.end(),
										  [uid](auto& test) { return test.getPropertyString("uid") == uid; });
				if (found != unorderedPanelContent.end()) {
					panel_content.addChild(*found);
				}
			}
		} else {
			auto unorderedPanelContent = panel_content.getChildren();
			std::sort(unorderedPanelContent.begin(), unorderedPanelContent.end(),
					  [](auto& a, auto& b) { return waffles::alphaSort(a, b); });
			panel_content.setChildren(unorderedPanelContent);
		}
		panel_content.setProperty("record_name", std::string(mFilterSelected));
		updatePanelContent(panel_content);
		filterButtonDown(mFilterSelected);
	});

	float startWidth  = mEngine.getWafflesSettings().getFloat("launcher:content_width", 0, 570.f);
	float startHeight = mEngine.getWafflesSettings().getFloat("launcher:content_height", 0, 700.f);

	BasePanel::setAbsoluteSizeLimits(ci::vec2(startWidth, startHeight), ci::vec2(startWidth, startHeight));

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);


	auto scroll_bar			= mPrimaryLayout->getSprite<ds::ui::ScrollBar>("side_scroll_bar");
	auto side_panel_content = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (scroll_bar && side_panel_content) {
		scroll_bar->linkScrollList(side_panel_content);
		scroll_bar->enableAutoHiding(mEngine.getWafflesSettings().getBool("launcher:scroller:auto_hide", 0, false));
	}

	if (auto back_button = mPrimaryLayout->getSprite<ds::ui::LayoutButton>("back_button")) {
		setBackButtonFn(back_button);
	}

	auto mainPanelScroll = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("content_holder");
	auto sidePanelScroll = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (mainPanelScroll && sidePanelScroll) {
		auto tappedCb = [this](ds::ui::SmartLayout* item, ds::model::ContentModelRef model) {
			buttonTapHandler(item, item->getGlobalPosition());
		};

		auto contentUpdatedCb = [this](ds::ui::SmartLayout* item) {
			updateItem(item);
			handleSelection();
		};

		auto selectionCb = [this](ds::ui::Sprite* bs, const bool highlighted) {
			updateSelection(bs, highlighted);
		};

		mainPanelScroll->setContentItemTappedCallback(tappedCb);
		sidePanelScroll->setContentItemTappedCallback(tappedCb);

		mainPanelScroll->setContentItemUpdatedCallback(contentUpdatedCb);
		sidePanelScroll->setContentItemUpdatedCallback(contentUpdatedCb);

		mainPanelScroll->setStateChangeCallback(selectionCb);
		sidePanelScroll->setStateChangeCallback(selectionCb);
	}

	if (auto sidePanelArrow = mPrimaryLayout->getSprite<ds::ui::ImageButton>("side_panel_arrow")) {
		sidePanelArrow->setTapCallback([this](ds::ui::Sprite* sp, const ci::vec3& pos) {
			mPanelHistory.pop_back();
			if (mPanelHistory.size() == 0) {
				closePanel();
			} else {
				updatePanelContent(mPanelHistory.back());
			}
		});
	}

	updateMenuItems();
	setupMenuItems();
	handleSelection();

	mEngine.getNotifier().notify(waffles::WafflesFilterEvent("Recent"));

	// TODO: idk why this is needed, but otherwise filter color starts wrong
	auto force_color = [this] {
		auto filter_holder = mPrimaryLayout->getSprite("fixed_buttons_bottom");
		if (!filter_holder) return;
		auto filter_buttons = filter_holder->getChildren();
		if (filter_buttons.empty()) return;
		auto text = filter_buttons[0]->getFirstDescendantWithName(L"name_high");
		auto icon = filter_buttons[0]->getFirstDescendantWithName(L"icon_high");
		if (!text || !icon) return;
		auto color = mEngine.getColors().getColorFromName("waffles:button:text:high:dark");
		text->setColor(color);
		icon->setColor(color);
	};
	force_color();

	/* auto alreadyPres = !mEngine.mContent.getChildByName("current_presentation").getChildren().empty();
	if (!alreadyPres) {
		auto pres_id = downstream::getInitialPresentation(mEngine);
		if (!pres_id.empty()) {
			auto pres = downstream::getRecordByUid(mEngine, pres_id);
			if (pres.getChildren().size() > 0) { // activate first slide
				mEngine.getNotifier().notify(RequestEngagePresentation(pres.getChild(0), false));
				mEngine.mContent.setProperty("presentation_controller_blocked", false);
			}
		}
	} */

	// these are to hide this from showing up in saved drawings
	mEventClient.listenToEvents<RequestPreDrawingSave>([this](auto& e) { hide(); });
	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) { show(); });

	mEngine.timedCallback(
		[this] {
			onLayout();
			closeButtonPlacement();
		},
		0.1f);
}

void Launcher::updateItem(ds::ui::SmartLayout* item) {
	if (!mPrimaryLayout) return;

	ContentUtils::configureListItem(mEngine, item, ci::vec2(mPrimaryLayout->getWidth(), item->getHeight()));
	item->setSize(mPrimaryLayout->getWidth(), item->getHeight());
	setButtonCallbacks(item);
}

void Launcher::updateSelection(ds::ui::Sprite* bs, const bool highlighted) {
	auto item = dynamic_cast<ds::ui::SmartLayout*>(bs);
	if (!item) return;
	auto btn = item->getSprite<ds::ui::LayoutButton>("the_btn");
	if (!btn) return;

	if (highlighted) {
		btn->showDown();
	} else {
		btn->showUp();
	}
}

void Launcher::handleSelection() {
	auto curPres		  = mEngine.mContent.getChildByName("current_presentation");
	auto interactive	  = curPres.getChild(0);
	auto interactiveSlide = interactive.getChild(curPres.getPropertyInt("current_slide") - 1);

	auto isSelected = [this, interactive, interactiveSlide](ds::model::ContentModelRef model) -> bool {
		return (model == mSelectedMain) || (mSelectedMain.empty() && model == interactive) ||
			   (model == interactiveSlide);
	};

	for (auto b : mMainButtons) {
		updateSelection(b, isSelected(b->getContentModel()));
	}
	if (auto smarty = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("content_holder")) {
		smarty->forEachLoadedSprite([this, isSelected](ds::ui::Sprite* sp) {
			auto b = dynamic_cast<ds::ui::SmartLayout*>(sp);
			if (!b) return;
			updateSelection(b, isSelected(b->getContentModel()));
		});
	}

	if (auto smarty = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content")) {
		smarty->forEachLoadedSprite([this, isSelected](ds::ui::Sprite* sp) {
			auto b = dynamic_cast<ds::ui::SmartLayout*>(sp);
			if (!b) return;
			updateSelection(b, isSelected(b->getContentModel()));
		});
	}
}

void Launcher::updateMenuItems() {

	auto tops_size = mEngine.getWafflesSettings().getVec2("ui:waffles:launcher:top:button:size", 0, ci::vec2(382.f, 114.f));

	auto topNames = std::vector<std::string>();
	auto configTop = mEngine.getWafflesSettings().getString("launcher:top:functions", 0, "Ambient,Search");
	topNames = ds::split(configTop, ",");
	auto top = std::vector<ds::model::ContentModelRef>();
	for (std::string name : topNames) {
		auto mode = ds::model::ContentModelRef(name);
		mode.setProperty("type_key", std::string(name));
		mode.setProperty("record_name", std::string(name));
		mode.setProperty("width_override", tops_size.x);
		mode.setProperty("height_override", tops_size.y);
		top.push_back(mode);
	}

	

	mNeedsTopRefresh	   = false;
	mNeedsScrollingRefresh = false;
	mNeedsBottomRefresh	   = false;


	//????? why check if just ambient is there?
	if (mMenuItemsTop != top) {
		mMenuItemsTop.clear();
		mMenuItemsTop	 = top;
		mNeedsTopRefresh = true;
	}

	auto leftSideNames = std::vector<std::string>();
	auto configLeftSide = mEngine.getWafflesSettings().getString(
		"launcher:main:filters", 0, "Recent|Recent,Images|Images,Links|Links,PDFs|PDFs,Presentations|Presentations,Streams|Streams,Videos|Videos,Folders|Folders");
	leftSideNames = ds::split(configLeftSide, ",");
	auto leftSide = std::vector<ds::model::ContentModelRef>();
	for (std::string btnConfig : leftSideNames) {
		auto parts = ds::split(btnConfig, "|");
		if (parts.size() != 2) {
			DS_LOG_WARNING("Invalid launcher button config: " << btnConfig);
			continue;
		}


		auto type = std::string(parts[1]);
		auto name = std::string(parts[0]);
		auto mode = ds::model::ContentModelRef(name);
		mode.setProperty("type_key", std::string(type));
		mode.setProperty("record_name", std::string(name));
		leftSide.push_back(mode);
	}

	if (mMenuItemsBottom != leftSide) {
		mMenuItemsBottom.clear();
		for (auto item : leftSide) {
			if (!item.empty()) {
				// item.setProperty("width_override", 795.f);
				mMenuItemsBottom.push_back(item);
			}
		}
		mNeedsBottomRefresh = true;
	}


	closeButtonPlacement();
}

void Launcher::setupMenuItems() {

	if (!mNeedsTopRefresh && !mNeedsScrollingRefresh && !mNeedsBottomRefresh) return;
	if (!mPrimaryLayout) return;

	mMainButtons.clear();

	if (auto toppy = mPrimaryLayout->getSprite<ds::ui::LayoutSprite>("fixed_buttons_top")) {
		toppy->clearChildren();
		for (auto btnContent : mMenuItemsTop) {
			auto btn = createButton(btnContent);
			mMainButtons.push_back(btn);
			if (auto btnBtn = btn->getSprite("the_btn")) {
				btnBtn->setTapCallback([this, btn](auto* sp, const auto& pos) { buttonTapHandler(btn, pos); });
			}
			toppy->addChildPtr(btn);
		}
	}

	if (mFirstFilterShove) {
		mFirstFilterShove = false;
		if (auto bot = mPrimaryLayout->getSprite<ds::ui::LayoutSprite>("fixed_buttons_bottom")) {
			bot->clearChildren();
			for (auto btnContent : mMenuItemsBottom) {
				auto btn = createButton(btnContent);
				// mMainButtons.push_back(btn);
				if (auto btnBtn = btn->getSprite("the_btn")) {
					btnBtn->setTapCallback([this, btn](auto* sp, const auto& pos) { buttonTapHandler(btn, pos); });
				}
				bot->addChildPtr(btn);
				mFilterButtons[btnContent.getPropertyString("type_key")] = btn;
			}
			mEngine.getNotifier().notify(waffles::WafflesFilterEvent(mFilterSelected));
		}
	}

	if (auto smarty = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("content_holder")) {
		auto itemHeight		 = mEngine.getWafflesSettings().getFloat("info_list:item:height", 0, 60.f);
		auto scrollMaxHeight = mEngine.getWafflesSettings().getFloat("launcher:scroll:height", 0, 375.f);
		if (mMenuItemsScrolling.size() * itemHeight < scrollMaxHeight) {
			smarty->setSize(smarty->getWidth(), mMenuItemsScrolling.size() * itemHeight);
			smarty->getScrollArea()->enableScrolling(false);
		} else {
			smarty->setSize(smarty->getWidth(), scrollMaxHeight);
			smarty->getScrollArea()->enableScrolling(true);
		}
		smarty->setContentList(mMenuItemsScrolling);
	}


	onLayout();
	closeButtonPlacement();
}

void Launcher::showSearch() {
}

void Launcher::onLayout() {
	if (!mPrimaryLayout) return;

	if (auto contHoldy = mPrimaryLayout->getSprite("content_holdery")) {
		if (mPanelOpen) { // required: app crashes without this check
			setSize(contHoldy->getSize());
		} else {
			setSize(contHoldy->getSize());
		}
	}
	mPrimaryLayout->setSize(mPrimaryLayout->getWidth(), getHeight());
	mPrimaryLayout->runLayout();
}

void Launcher::onCreationArgsSet() {
	mMaxViewersOfThisType = 1;
}


ds::ui::SmartLayout* Launcher::createButton(ds::model::ContentModelRef item) {
	auto assetBtn = new ds::ui::SmartLayout(mEngine, "waffles/common/filter_item.xml");
	assetBtn->setContentModel(item);
	updateItem(assetBtn);
	assetBtn->enable(false);
	if (auto btn = assetBtn->getSprite("the_btn")) {
		btn->enable(true);
		auto right_bar = assetBtn->getSprite("right_bar");
		if (!item.getProperty("width_override").empty()) {
			auto value = item.getProperty("width_override").getFloat();
			assetBtn->setSize(ci::vec2(value, assetBtn->getHeight()));
			if (right_bar) right_bar->mLayoutLPad = value - 102.f * mWafflesScale;
			btn->setSize(ci::vec2(value, btn->getHeight()));
		}
		if (!item.getProperty("height_override").empty()) {
			auto value = item.getProperty("height_override").getFloat();
			assetBtn->setSize(ci::vec2(assetBtn->getWidth(), value));
			if (right_bar) right_bar->setSize(ci::vec2(right_bar->getWidth(), value));
			btn->setSize(ci::vec2(btn->getWidth(), value));
		}
	}
	assetBtn->runLayout();
	return assetBtn;
}

void Launcher::setButtonCallbacks(ds::ui::SmartLayout* assetBtn) {
	if (auto theBtn = assetBtn->getSprite("the_btn")) {
		theBtn->setTapCallback([this, assetBtn](auto* sp, const auto& pos) { buttonTapHandler(assetBtn, pos); });
	}
}

void Launcher::buttonTapHandler(ds::ui::Sprite* sp, const ci::vec3& pos) {
	auto btn = dynamic_cast<ds::ui::SmartLayout*>(sp);
	if (!btn) return;
	updateRecent(btn->getContentModel());
	auto offset = mEngine.getWafflesSettings().getVec3("launcher:media_open:offset", 0, ci::vec3(1000.f, 0, 0)) *
				  ((pos.x > (mEngine.getWorldWidth() / 2.f)) ? ci::vec3(-1.f, 1.f, 1.f) : ci::vec3(1.f, 1.f, 1.f));
	bool alreadyHandled = ContentUtils::handleListItemTap(mEngine, btn, pos + offset);
	if (alreadyHandled) return;

	auto model	 = btn->getContentModel();
	auto type	 = model.getPropertyString("type_key");
	auto typeUid = model.getPropertyString("type_uid");

	if (ContentUtils::getDefault(mEngine)->isFolder(model) || ContentUtils::getDefault(mEngine)->isPresentation(model)) {
		panelButtonTapped(btn);
	} else if (type == "presentation") {
		if (model.getChildren().size() > 0) { // activate first slide
			mEngine.getNotifier().notify(RequestEngagePresentation(model.getChild(0)));
			mEngine.mContent.setProperty("presentation_controller_blocked", false);
		}
	} else if (type == "current_playlist") {
		panelButtonTapped(btn);
	} else if (type == "pinboard") {
		panelButtonTapped(btn);
	} else if (type == "streams") {
		panelButtonTapped(btn);
	} else {
		mEngine.getNotifier().notify(RequestEngagePresentation(model));
		DS_LOG_INFO("Possibly unhandled menu item! " << type << " : " << model.getPropertyString("record_name") << " ("
													 << type << ")");
	}
}

void Launcher::panelButtonTapped(ds::ui::SmartLayout* button) {
	if (!mPrimaryLayout) return;
	auto sidePanelContent = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (!sidePanelContent) return;

	auto model		   = button->getContentModel();
	bool fromMainPanel = false;
	for (auto& group : {mMenuItemsTop, mMenuItemsScrolling, mMenuItemsBottom}) {
		for (auto item : group) {
			if (item == model) {
				fromMainPanel = true;
				break;
			}
		}
	}
	if (fromMainPanel) {
		mPanelHistory.clear();
	}

	mPanelHistory.push_back(model);
	updatePanelContent(model);

	checkBounds(false);
}

void Launcher::updatePanelContent(ds::model::ContentModelRef model) {
	if (!mPrimaryLayout) return;
	auto sidePanel		  = mPrimaryLayout->getSprite("side_panel");
	auto sidePanelContent = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (!sidePanel || !sidePanelContent) return;
	/*
	if (mPanelOpen && !mPanelTransitioning) {
		sidePanelContent->setOpacity(0.f);
		updatePanelContent(model);
		mPanelTransitioning = true;
		return;
	}
	*/
	auto mainWidth = mPrimaryLayout->getWidth();

	mPrimaryLayout->setSpriteText("side_panel_title", model.getPropertyString("record_name"));

	auto filteredModel = ds::model::ContentModelRef(model.getName());
	filteredModel.setProperties(model.getProperties());
	for (auto child : model.getChildren()) {
		if (child.getPropertyString("type_key") != "ambient_playlist") {
			filteredModel.addChild(child);
		}
	}

	sidePanelContent->setContentList(filteredModel.getChildren());

	sidePanel->show();

	handleSelection();

	if (!mPanelOpen) {
		sidePanel->setPosition(ci::vec3(795.f, 150.f, 0.f));
		mPanelOpen = true;
	} else if (mPanelTransitioning) {
		sidePanelContent->setOpacity(1.f);
		callAfterDelay([sidePanel, mainWidth] { sidePanel->setPosition(ci::vec3(795.f, 150.f, 0.f)); }, 0.01f);
		mPanelTransitioning = false;
	}

	onLayout();
}

bool Launcher::unrepeatedContent(ds::model::ContentModelRef existing, ds::model::ContentModelRef addition) {
	auto addition_uid = addition.getPropertyString("uid");
	for (auto item : existing.getChildren()) {
		if (item.getPropertyString("uid") == addition_uid) {
			return false;
		}
	}
	return true;
}

bool Launcher::filterValid(std::string type, ds::model::ContentModelRef model) {
	if (type == "Images") {
		return ContentUtils::getDefault(mEngine)->isMedia(model) &&
			   model.getPropertyResource("media").getType() == ds::Resource::IMAGE_TYPE;
	} else if (type == "Presentations") {
		return ContentUtils::getDefault(mEngine)->isPresentation(model);
	} else if (type == "Videos") {
		return ContentUtils::getDefault(mEngine)->isMedia(model) &&
			   (model.getPropertyResource("media").getType() == ds::Resource::VIDEO_TYPE ||
				model.getPropertyResource("media").getType() == ds::Resource::YOUTUBE_TYPE);
	} else if (type == "Streams") { // TODO: untested
		return ContentUtils::getDefault(mEngine)->isMedia(model) &&
			   model.getPropertyResource("media").getType() == ds::Resource::VIDEO_STREAM_TYPE;
	} else if (type == "PDFs") { // TODO: untested
		return ContentUtils::getDefault(mEngine)->isMedia(model) &&
			   model.getPropertyResource("media").getType() == ds::Resource::PDF_TYPE;
	} else if (type == "Links") {
		return ContentUtils::getDefault(mEngine)->isMedia(model) &&
			   model.getPropertyResource("media").getType() == ds::Resource::WEB_TYPE;
	} else if (type == "Recent") {
		loadRecent();
		return recentContains(model);
	} else if (type == "Folders") {
		return ContentUtils::getDefault(mEngine)->isFolder(model);
	}
	return false;
}

void Launcher::updateRecent(ds::model::ContentModelRef content) {
	if (std::find(mFolderStack.begin(), mFolderStack.end(), content) == mFolderStack.end() &&
		(content.getPropertyString("type_key") == waffles::MEDIA_TYPE_DIRECTORY_CMS || ContentUtils::getDefault(mEngine)->isFolder(content))) {
		mFolderStack.push_back(content);
		if (auto back_button = mPrimaryLayout->getSprite("back_button")) {
			back_button->show();
		}
	}
	auto uid = content.getPropertyString("uid");
	loadRecent();
	if (recentContains(content)) { // if already in recent list
		mRecentFilterUids.erase(std::find(mRecentFilterUids.begin(), mRecentFilterUids.end(),
										  uid)); // remove from list before adding to front
	}
	mRecentFilterUids.insert(mRecentFilterUids.begin(), uid); // add to front
	if (mRecentFilterUids.size() > mEngine.getWafflesSettings().getInt("waffles:recent:max", 0, 10)) {
		mRecentFilterUids.pop_back(); // remove last item, if hit max
	}
	saveRecent();
}

void Launcher::loadRecent() {
	if (mRecentFileHandling) return;
	mRecentFileHandling = true;
	std::string	  contents;
	std::ifstream recent_file(getRecentFilePath());
	if (recent_file.is_open() && recent_file.good()) {
		recent_file >> contents;
	}
	recent_file.close();
	auto uids = ds::split(contents, ",", true);
	mRecentFilterUids.clear();
	mRecentFilterUids	= uids;
	mRecentFileHandling = false;
}

void Launcher::saveRecent() {
	if (mRecentFileHandling) return;
	mRecentFileHandling = true;
	std::string recents = "";
	for (auto uid : mRecentFilterUids) {
		recents += uid + ",";
	}
	std::ofstream recent_file(getRecentFilePath());
	recent_file << recents;
	recent_file.close();
	mRecentFileHandling = false;
}

void Launcher::filterButtonDown(std::string type) {
	auto filter_names =
		std::vector<std::string>{"Recent", "Images", "Links", "PDFs", "Presentations", "Streams", "Videos", "Folders"};
	auto normal_bg			= mEngine.getColors().getColorFromName("waffles:button:bg:normal:dark");
	auto high_bg			= mEngine.getColors().getColorFromName("waffles:button:bg:high:dark");
	auto normal_text		= mEngine.getColors().getColorFromName("waffles:button:text:normal:dark");
	auto high_text			= mEngine.getColors().getColorFromName("waffles:button:text:high:dark");
	auto force_button_state = [normal_bg, high_bg, normal_text, high_text](ds::ui::SmartLayout*	 layout,
																		   ds::ui::LayoutButton* button, bool up) {
		if (!(layout && button)) return;
		up ? button->showUp() : button->showDown();
		layout->getSprite("background_highlight")->setColor(up ? normal_bg : high_bg);
		layout->getSprite("background_highlight_high")->setColor(up ? normal_bg : high_bg);
		layout->getSprite("icon")->setColor(up ? normal_text : high_text);
		layout->getSprite("icon_high")->setColor(up ? high_text : normal_text);
		layout->getSprite("name")->setColor(up ? normal_text : high_text);
		layout->getSprite("name_high")->setColor(up ? high_text : normal_text);
	};
	for (auto name : filter_names) {
		if (mFilterButtons.count(name) > 0) {
			auto button_layout = mFilterButtons[name];
			if (button_layout) {
				auto button_sprite = button_layout->getSprite("the_btn");
				if (button_sprite) {
					auto button = dynamic_cast<ds::ui::LayoutButton*>(button_sprite);
					if (button) {
						bool up = name != type;
						force_button_state(button_layout, button, up);
						button->enable(up);
					}
				}
			}
		}
	}
}

void Launcher::closeButtonPlacement() {
	if (auto closeBtn = mPrimaryLayout->getSprite("close_button.the_button")) {
		ci::vec3 fudge;
		if (mFirstCloseButton) {
			mFirstCloseButton  = false;
			mSecondCloseButton = true;
			fudge			   = mEngine.getWafflesSettings().getVec3("launcher:close:start:offset", 0, ci::vec3(0, 0, 0));
		} else {
			mSecondCloseButton = false;
			fudge			   = mEngine.getWafflesSettings().getVec3("launcher:close:normal:offset", 0, ci::vec3(0, 0, 0));
		}
		closeBtn->mLayoutFudge = fudge;
	}
}

void Launcher::setBackButtonFn(ds::ui::LayoutButton* button) {
	button->setClickFn([this] {
		if (mFolderStack.size() > 1) {
			updatePanelContent(mFolderStack[mFolderStack.size() - 2]);
			mFolderStack.pop_back();
		} else if (mFilterSelected == "Folders" || mFilterSelected == "Recent") {
			auto filter		= mFilterSelected;
			mFilterSelected = ""; // to let the next thing happen
			mEngine.getNotifier().notify(waffles::WafflesFilterEvent(filter));
		}
	});
}

std::vector<ds::model::ContentModelRef> Launcher::recurseContent(std::vector<ds::model::ContentModelRef> the_content) {
	std::vector<ds::model::ContentModelRef> out_content;
	for (auto child : the_content) {
		if (child.getPropertyString("type_key") == waffles::MEDIA_TYPE_DIRECTORY_CMS) {
			for (auto sub : recurseContent(child.getChildren())) {
				out_content.push_back(sub);
			}
		}
		out_content.push_back(child);
	}
	return out_content;
}

void Launcher::closePanel() {
	return; // cannot close panel in HPI
	auto sidePanel = mPrimaryLayout->getSprite("side_panel");
	if (!sidePanel) return;

	if (mPanelOpen) {
		sidePanel->tweenPosition(ci::vec3(0.f), 0.25f, 0.0f, ci::easeInQuad, [this, sidePanel] {
			mPanelOpen = false;
			onLayout();
			sidePanel->hide();
		});
	}
}

} // namespace waffles
