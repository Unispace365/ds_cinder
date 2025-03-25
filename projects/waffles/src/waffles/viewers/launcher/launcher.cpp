#include "stdafx.h"

#include "launcher.h"

#include <fstream>
#include <iostream>
#include <utility>

#include <ds/app/engine/engine_roots.h>
#include <ds/app/environment.h>
#include <ds/content/content_events.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"
#include "waffles/common/ui_utils.h"
#include "waffles/query/search_query.h"
#include "waffles/util/waffles_helper.h"
#include "waffles/waffles_events.h"

// using namespace downstream;

namespace waffles {

Launcher::Launcher(ds::ui::SpriteEngine& g, std::string eventChannel, bool hideClose)
  : BaseElement(g, std::move(eventChannel)) {

	mEventClient.notify(WafflesLauncherOpened(this));

	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_LAUNCHER;
	mCanResize			  = true;
	mCanArrange			  = false;

	mPrimaryLayout			 = new ds::ui::SmartLayout(mEngine, "waffles/launcher/launcher.xml");
	mWafflesScale			 = mEngine.getWafflesSettings().getFloat("waffles:sprite:scale", 0, 1.f);
	mCloseOnViewerFullscreen = mEngine.getWafflesSettings().getBool("launcher:close_when_viewers_fullscreen", 0, true);
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
	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](const auto& ev) {
		callAfterDelay(
			[this] {
				updateMenuItems();
				setupMenuItems();
				handleSelection();
				mEventClient.notify(WafflesFilterEvent(mFilterSelected, false));
			},
			0.01f);
	});

	mEventClient.listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) {
		callAfterDelay(
			[this] {
				updateMenuItems();
				setupMenuItems();
				handleSelection();
				mEventClient.notify(WafflesFilterEvent(mFilterSelected, false));
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

	mEventClient.listenToEvents<RequestPresentationAdvanceEvent>(
		[this](const auto& ev) { callAfterDelay([this] { handleSelection(); }, 0.01f); });

	mEventClient.listenToEvents<TemplateChangeComplete>([this](const auto& ev) { activatePanel(); });
	mEventClient.listenToEvents<RequestFullscreenViewer>([this](const auto& e) {
		if (mCloseRequestCallback && mCloseOnViewerFullscreen) mCloseRequestCallback();
	});
	mEventClient.listenToEvents<WafflesFilterEvent>([this](const WafflesFilterEvent& ev) {
		auto helper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();

		mFilterSelected = ev.mType;
		DS_LOG_INFO("Waffles filtering by '" << mFilterSelected << "'.");

		auto savedStack = mFolderStack;
		mFolderStack.clear();
		if (auto back_button = mPrimaryLayout->getSprite("back_button")) {
			back_button->hide();
		}

		std::vector<ds::model::ContentModelRef> allContent;

		auto pinny = helper->getPinboard();
		if (!pinny.empty()) allContent.push_back(pinny);

		auto allValid = helper->getContentForPlatform();

		for (auto& value : allValid) {
			allContent.push_back(value);
		}

		auto non_recursive = ds::split(
			mEngine.getWafflesSettings().getString("launcher:non-recursive:filters", 0, "recent,folders"), ",");
		if (std::find(non_recursive.begin(), non_recursive.end(), mFilterSelected) == non_recursive.end()) {
			allContent = recurseContent(allContent);
		}

		auto panel_content = ds::model::ContentModelRef();
		panel_content.setName(mFilterSelected);
		for (const auto& content : allContent) {
			if (ds::model::ContentHelperFactory::getDefault<WafflesHelper>()->isValidForFilter(mFilterSelected,
																							   content) &&
				unrepeatedContent(panel_content, content)) {
				panel_content.addChild(content);
			}
		}

		auto auto_expandable =
			ds::split(mEngine.getWafflesSettings().getString("launcher:auto-expandable:filters", 0, ""), ",");
		if (std::find(auto_expandable.begin(), auto_expandable.end(), mFilterSelected) != auto_expandable.end() &&
			panel_content.getChildren().size() == 1 &&
			ContentUtils::getDefault(mEngine)->isFolder(panel_content.getChildren()[0])) {
			auto content = panel_content.getChildren()[0].getChildren();
			panel_content.clearChildren();
			for (const auto& child : content) {
				panel_content.addChild(child);
			}
		}


		// recent files need to be sorted correctly, not by all content ordering :(
		if (mFilterSelected == "recent") {
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
			panel_content.setChildren(unorderedPanelContent);
		}
		panel_content.setProperty("record_name", std::string(mFilterSelected));
		updatePanelContent(panel_content);
		filterButtonDown(mFilterSelected);
		for (const std::string& name :
			 ds::split(mEngine.getWafflesSettings().getString("launcher:filter_labels:sprite_names", 0, ""), ",")) {
			mPrimaryLayout->setSpriteText(name, upperedFilterText());
		}

		// Try to get back to the same subfolder/whatever if this is coming from a content refresh
		if (!ev.mFromButton && !savedStack.empty()) {
			auto currentSubContent = panel_content;
			for (auto crumb : savedStack) {
				for (auto content : currentSubContent.getChildren()) {
					if (content.getPropertyString("uid") == crumb.getPropertyString("uid")) {
						currentSubContent = content;
						break;
					}
				}

				if (currentSubContent.getPropertyString("uid") == crumb.getPropertyString("uid")) {
					// mPanelHistory.push_back(currentSubContent);
					mFolderStack.push_back(currentSubContent);
					updatePanelContent(currentSubContent);
				} else {
					break;
				}
			}

			if (mFolderStack.size() > 0) {
				if (auto back_button = mPrimaryLayout->getSprite("back_button")) {
					back_button->show();
				}
			}
		}
		updateBreadcrumbText();
		onFilterChanged();
	});

	float startWidth  = mEngine.getWafflesSettings().getFloat("launcher:content_width", 0, 570.f);
	float startHeight = mEngine.getWafflesSettings().getFloat("launcher:content_height", 0, 700.f);

	setAbsoluteSizeLimits(ci::vec2(startWidth, startHeight), ci::vec2(startWidth, startHeight));

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
		auto tappedCb = [this](ds::ui::SmartLayout* item, const ds::model::ContentModelRef& model) {
			buttonTapHandler(item, item->getGlobalPosition());
		};

		auto contentUpdatedCb = [this](ds::ui::SmartLayout* item) {
			updateItem(item);
			handleSelection();
		};

		auto selectionCb = [this](Sprite* bs, const bool highlighted) {
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
		sidePanelArrow->setTapCallback([this](Sprite* sp, const ci::vec3& pos) {
			mPanelHistory.pop_back();
			if (mPanelHistory.empty()) {
				closePanel();
			} else {
				updatePanelContent(mPanelHistory.back());
			}
		});
	}

	updateMenuItems();
	setupMenuItems();
	handleSelection();


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


	// these are to hide this from showing up in saved drawings
	mEventClient.listenToEvents<RequestPreDrawingSave>([this](auto& e) { hide(); });
	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) { show(); });

	auto helper				= ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	auto customFilters		= helper->getLauncherCustomFilters();
	customFilters["recent"] = [this](ds::model::ContentModelRef model) {
		loadRecent();
		return recentContains(std::move(model));
	};
	helper->setLauncherCustomFilters(customFilters);

	mEngine.timedCallback(
		[this] {
			onLayout();
			closeButtonPlacement();
		},
		0.1);
}

void Launcher::updateItem(ds::ui::SmartLayout* item) {
	if (!mPrimaryLayout) return;

	auto sidePanelScroll = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (sidePanelScroll) {
		ContentUtils::configureListItem(mEngine, item, ci::vec2(sidePanelScroll->getWidth(), item->getHeight()),
										mEnableSelection);
		item->setSize(sidePanelScroll->getWidth(), item->getHeight());
	}
	setButtonCallbacks(item);
}

void Launcher::updateSelection(Sprite* bs, const bool highlighted) {
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
	auto curPres		  = mEngine.mContent.getChildByName("current_presentation" + getChannelName());
	auto interactive	  = curPres.getChild(0);
	auto interactiveSlide = interactive.getChild(curPres.getPropertyInt("current_slide") - 1);

	auto isSelected = [this, interactive, interactiveSlide](const ds::model::ContentModelRef& model) -> bool {
		return (model == mSelectedMain) || (mSelectedMain.empty() && model == interactive) ||
			   (model == interactiveSlide);
	};

	for (auto b : mMainButtons) {
		updateSelection(b, isSelected(b->getContentModel()));
	}
	if (auto smarty = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("content_holder")) {
		smarty->forEachLoadedSprite([this, isSelected](Sprite* sp) {
			auto b = dynamic_cast<ds::ui::SmartLayout*>(sp);
			if (!b) return;
			updateSelection(b, isSelected(b->getContentModel()));
		});
	}

	if (auto smarty = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content")) {
		smarty->forEachLoadedSprite([this, isSelected](Sprite* sp) {
			auto b = dynamic_cast<ds::ui::SmartLayout*>(sp);
			if (!b) return;
			updateSelection(b, isSelected(b->getContentModel()));
		});
	}
}

ds::model::ContentModelRef Launcher::buttonCfgFromString(const std::string& str) const {
	auto parts = ds::split(str, "|");
	if (parts.size() < 2) {
		DS_LOG_WARNING("Invalid launcher button config: " << str);
		return {};
	}
	auto type = std::string(parts[1]);
	auto name = std::string(parts[0]);
	auto mode = ds::model::ContentModelRef();
	mode.setName(name);
	mode.setProperty("type_key", std::string(type));
	mode.setProperty("record_name", std::string(name));
	mode.setProperty("has_icon", false);
	if (parts.size() > 2) {
		auto res = ds::Resource::fromImage(ds::Environment::expand(parts[2]));
		if (!res.empty()) {
			mode.setPropertyResource("icon_src", res);
			mode.setProperty("has_icon", true);
		}
	}
	return mode;
}

void Launcher::updateMenuItems() {

	auto tops_size =
		mEngine.getWafflesSettings().getVec2("ui:waffles:launcher:top:button:size", 0, ci::vec2(382.f, 114.f));

	auto topNames = std::vector<std::string>();
	auto configTop =
		mEngine.getWafflesSettings().getString("launcher:top:functions", 0, "Ambient|ambient,Search|search");
	topNames = ds::split(configTop, ",");
	auto top = std::vector<ds::model::ContentModelRef>();
	for (const std::string& btnConfig : topNames) {
		auto mode = buttonCfgFromString(btnConfig);
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
	auto configLeftSide =
		mEngine.getWafflesSettings().getString("launcher:main:filters", 0,
											   "Recent|Recent,Images|Images,Links|Links,PDFs|PDFs,Presentations|"
											   "Presentations,Streams|Streams,Videos|Videos,Folders|Folders");
	leftSideNames = ds::split(configLeftSide, ",");
	auto leftSide = std::vector<ds::model::ContentModelRef>();
	for (const std::string& btnConfig : leftSideNames) {
		auto mode = buttonCfgFromString(btnConfig);
		leftSide.push_back(mode);
	}

	if (mMenuItemsBottom != leftSide) {
		mMenuItemsBottom.clear();
		for (const auto& item : leftSide) {
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
		for (const auto& btnContent : mMenuItemsTop) {
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
			for (const auto& btnContent : mMenuItemsBottom) {
				auto btn = createButton(btnContent);
				// mMainButtons.push_back(btn);
				if (auto btnBtn = btn->getSprite("the_btn")) {
					btnBtn->setTapCallback([this, btn](auto* sp, const auto& pos) { buttonTapHandler(btn, pos); });
				}
				bot->addChildPtr(btn);
				mFilterButtons[btnContent.getPropertyString("type_key")] = btn;
			}
			mEventClient.notify(WafflesFilterEvent(mFilterSelected));
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

void Launcher::showSearch() {}

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

void Launcher::onParentSet() {
	BaseElement::onParentSet();
	if (mParent) {
		mEngine.timedCallback(
			[this] {
				auto starting_filter = mEngine.getWafflesSettings().getString("launcher:start:filter", 0, "recent");
				mEventClient.notify(WafflesFilterEvent(starting_filter));
				closeButtonPlacement();
			},
			0.001);
	}
}


ds::ui::SmartLayout* Launcher::createButton(ds::model::ContentModelRef item) {
	auto assetBtn = new ds::ui::SmartLayout(mEngine, "waffles/common/filter_item.xml");
	assetBtn->setContentModel(item);
	updateItem(assetBtn);
	filterItemIconHandle(assetBtn);
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

void Launcher::buttonTapHandler(Sprite* sp, const ci::vec3& pos) {
	auto btn = dynamic_cast<ds::ui::SmartLayout*>(sp);
	if (!btn) return;
	updateRecent(btn->getContentModel());
	auto offset = mEngine.getWafflesSettings().getVec3("launcher:media_open:offset", 0, ci::vec3(1000.f, 0, 0)) *
				  ((pos.x > (mEngine.getWorldWidth() / 2.f)) ? ci::vec3(-1.f, 1.f, 1.f) : ci::vec3(1.f, 1.f, 1.f));
	bool alreadyHandled = ContentUtils::handleListItemTap(mEngine, btn, getChannelName(), pos + offset, pos);
	if (alreadyHandled) return;

	auto model	 = btn->getContentModel();
	auto type	 = model.getPropertyString("type_key");
	auto typeUid = model.getPropertyString("type_uid");

	if (ContentUtils::getDefault(mEngine)->isFolder(model)) {
		panelButtonTapped(btn);
	} else if (type == "presentation" || ContentUtils::getDefault(mEngine)->isPresentation(model)) {
		panelButtonTapped(btn); // see ui_utils.cpp for presentation trigger
	} else if (type == "current_playlist") {
		panelButtonTapped(btn);
	} else if (type == "pinboard") {
		panelButtonTapped(btn);
	} else if (type == "streams") {
		panelButtonTapped(btn);
	} else {
		DS_LOG_INFO("Unhandled menu item! " << type << " : " << model.getPropertyString("record_name") << " (" << type
											<< ")");
	}
}

void Launcher::panelButtonTapped(ds::ui::SmartLayout* button) {
	if (!mPrimaryLayout) return;
	auto sidePanelContent = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("side_panel_content");
	if (!sidePanelContent) return;

	auto model		   = button->getContentModel();
	bool fromMainPanel = false;
	for (auto& group : {mMenuItemsTop, mMenuItemsScrolling, mMenuItemsBottom}) {
		for (const auto& item : group) {
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

void Launcher::updatePanelContent(const ds::model::ContentModelRef& model) {
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

	bool restrictive   = mEngine.getWafflesSettings().getBool("launcher:restrictive:enabled", 0, true);
	auto filteredModel = ds::model::ContentModelRef();
	filteredModel.setName(model.getName());
	filteredModel.setProperties(model.getProperties());
	for (const auto& child : model.getChildren()) {
		if (restrictive && !restrictiveType(child)) continue;
		filteredModel.addChild(child);
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

	onPanelContentUpdated();
	onLayout();
}

bool Launcher::unrepeatedContent(const ds::model::ContentModelRef& existing,
								 const ds::model::ContentModelRef& addition) {
	auto addition_uid = addition.getPropertyString("uid");
	for (const auto& item : existing.getChildren()) {
		if (item.getPropertyString("uid") == addition_uid) {
			return false;
		}
	}
	return true;
}

bool Launcher::restrictiveType(const ds::model::ContentModelRef& model) const {
	return ContentUtils::getDefault(mEngine)->isMedia(model) ||
		   ContentUtils::getDefault(mEngine)->isPresentation(model) ||
		   ContentUtils::getDefault(mEngine)->isFolder(model);
}

void Launcher::updateRecent(const ds::model::ContentModelRef& model) {
	if (std::find(mFolderStack.begin(), mFolderStack.end(), model) == mFolderStack.end() &&
		(model.getPropertyString("type_key") == MEDIA_TYPE_DIRECTORY_CMS ||
		 ContentUtils::getDefault(mEngine)->isFolder(model))) {
		mFolderStack.push_back(model);
		updateBreadcrumbText();
		if (auto back_button = mPrimaryLayout->getSprite("back_button")) {
			back_button->show();
		}
	}
	auto uid = model.getPropertyString("uid");
	loadRecent();
	if (recentContains(model)) { // if already in recent list
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
	std::string recents;
	for (const auto& uid : mRecentFilterUids) {
		recents += uid + ",";
	}
	std::ofstream recent_file(getRecentFilePath());
	recent_file << recents;
	recent_file.close();
	mRecentFileHandling = false;
}

void Launcher::filterButtonDown(const std::string& type) {
	// auto normal_bg	 = mEngine.getColors().getColorFromName("waffles:button:bg:normal:dark");
	// auto high_bg	 = mEngine.getColors().getColorFromName("waffles:button:bg:high:dark");
	// auto normal_text = mEngine.getColors().getColorFromName("waffles:button:text:normal:dark");
	// auto high_text	 = mEngine.getColors().getColorFromName("waffles:button:text:high:dark");

	auto force_button_state = [](const ds::ui::SmartLayout* layout, const ds::ui::LayoutButton* button, bool up) {
		if (!(layout && button)) return;
		up ? button->showUp() : button->showDown();
	};

	for (const auto& filter : mFilterButtons) {
		auto button_layout = mFilterButtons[filter.first];
		if (button_layout) {
			auto button_sprite = button_layout->getSprite("the_btn");
			if (button_sprite) {
				auto button = dynamic_cast<ds::ui::LayoutButton*>(button_sprite);
				if (button) {
					bool up = filter.first != type;
					// Since this is triggered during a button tap we need to wait for the next frame before forcing the
					// button state, otherwise the button interaction handler overwrites our change
					button->callAfterDelay(
						[button, up, button_layout, force_button_state] {
							button->enable(up);
							force_button_state(button_layout, button, up);
						},
						0.01f);
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
			fudge = mEngine.getWafflesSettings().getVec3("launcher:close:start:offset", 0, ci::vec3(0, 0, 0));
		} else {
			mSecondCloseButton = false;
			fudge = mEngine.getWafflesSettings().getVec3("launcher:close:normal:offset", 0, ci::vec3(0, 0, 0));
		}
		closeBtn->mLayoutFudge = fudge;
	}
}

void Launcher::setBackButtonFn(ds::ui::LayoutButton* button) {
	button->setClickFn([this] {
		std::vector<std::string> folder_enabled_filters =
			ds::split(mEngine.getWafflesSettings().getString("launcher:folder_enabled:filters", 0,
															 "Folders,folders,Recent,recent"),
					  ",");
		bool folder_enabled = false;
		for (const std::string& filter : folder_enabled_filters) {
			if (filter == mFilterSelected) {
				folder_enabled = true;
				break;
			}
		}
		if (mFolderStack.size() > 1) {
			updatePanelContent(mFolderStack[mFolderStack.size() - 2]);
			mFolderStack.pop_back();
			updateBreadcrumbText();
		} else if (folder_enabled) {
			auto filter		= mFilterSelected;
			mFilterSelected = ""; // to let the next thing happen
			mEventClient.notify(WafflesFilterEvent(filter, true));
		}
	});
}

std::vector<ds::model::ContentModelRef>
Launcher::recurseContent(const std::vector<ds::model::ContentModelRef>& content) {
	std::vector<ds::model::ContentModelRef> result;
	for (auto& child : content) {
		if (ContentUtils::getDefault(mEngine)->isFolder(child)) {
			for (const auto& sub : recurseContent(child.getChildren())) {
				result.push_back(sub);
			}
		}
		result.push_back(child);
	}
	return result;
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

void Launcher::updateBreadcrumbText() {
	if (!mPrimaryLayout) return;
	auto breadcrumb = mPrimaryLayout->getSprite<ds::ui::Text>("breadcrumb");
	if (!breadcrumb) return;
	std::string				 filter		= upperedFilterText();
	std::vector<std::string> text_stack = {};
	for (const auto& folder : mFolderStack) {
		text_stack.push_back(folder.getPropertyString("record_name"));
	}
	breadcrumb->setText(filter + " / " + ds::join(text_stack, " / "));
	auto max_width = mEngine.getWafflesSettings().getFloat("launcher:breadcrumb:max_width", 0, 500.f);
	while (breadcrumb->getWidth() > max_width && !text_stack.empty()) {
		text_stack.erase(text_stack.begin());
		breadcrumb->setText(filter + " / ... / " + ds::join(text_stack, " / "));
	}
	if (text_stack.empty()) {
		breadcrumb->setText(
			mEngine.getWafflesSettings().getBool("launcher:breadcrumb:root_says_filter", 0, true) ? filter : "");
		if (!mFolderStack.empty()) {
			DS_LOG_INFO("failed to fit waffles launcher breadcrumb text in width of " << max_width);
		}
	}
}

std::string Launcher::upperedFilterText() {
	std::string filter = mFilterSelected;
	if (!filter.empty()) {
		filter[0] = std::toupper(filter[0]);
	}
	return filter;
}

void Launcher::filterItemIconHandle(ds::ui::SmartLayout* item) {
	auto model = item->getContentModel();
	for (std::string name : {"icon", "icon_high"}) {
		if (auto icon = item->getSprite<ds::ui::Image>(name)) {
			if (model.getPropertyBool("has_icon")) {
				icon->setImageResource(model.getPropertyResource("icon_src"));
				icon->show();
			} else {
				icon->hide();
			}
		}
	}
}

} // namespace waffles
