#include "stdafx.h"

#include "search_viewer.h"

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"
#include "waffles/common/ui_utils.h"

namespace waffles {

SearchViewer::SearchViewer(ds::ui::SpriteEngine& g, const std::string& searchType)
	: BaseElement(g)
	, mPrimaryLayout(nullptr)
	, mSearchQuery(g, []() { return new SearchQuery(); })
	, mResourceFilter(0) {

	mViewerType			  = searchType;
	mMaxViewersOfThisType = 1;
	mCanResize			  = false;
	mCanArrange			  = false;
	mCanFullscreen		  = false;

	mSearchQuery.setReplyHandler([this](SearchQuery& q) { onSearchResults(q); });

	mPrimaryLayout = new ds::ui::SmartLayout(mEngine, "waffles/search/search_viewer.xml");
	addChildPtr(mPrimaryLayout);

	auto title = mPrimaryLayout->getSprite<ds::ui::Text>("title");
	if (title) {
		if (mViewerType == VIEW_TYPE_SEARCH) {
			title->setText("Search");
		} else if (mViewerType == VIEW_TYPE_SELECT_MEDIA_AMBIENT) {
			title->setText("Select Ambient Mode Media");
		} else if (mViewerType == VIEW_TYPE_SELECT_MEDIA_BACKGROUND) {
			title->setText("Select Background Media");
		}
	}
	auto fileList = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) {
		DS_LOG_WARNING("No file scroll list found for search viewer");
		return;
	}

	fileList->setContentItemTappedCallback([this](ds::ui::SmartLayout* sl, ds::model::ContentModelRef theModel) {
		bool handled = ContentUtils::handleListItemTap(mEngine, sl, getChannelName(), sl->getGlobalPosition());
		if (!handled) {
			DS_LOG_INFO("Possibly unhandled item in search_viewer: " << theModel.getPropertyString("type_key") << " : "
																	 << theModel.getPropertyString("record_name"));
		}
	});

	fileList->setContentItemUpdatedCallback([this, fileList](ds::ui::SmartLayout* sl) {
		ContentUtils::configureListItem(mEngine, sl, ci::vec2(fileList->getWidth(), sl->getHeight()));
	});

	fileList->setStateChangeCallback([this](ds::ui::Sprite* sp, const bool highlighted) {
		auto theSl = dynamic_cast<ds::ui::SmartLayout*>(sp);
		if (!theSl) return;
		if (auto btn = theSl->getSprite<ds::ui::LayoutButton>("the_btn")) {
			if (highlighted) {
				btn->callAfterDelay([btn] { btn->showDown(); }, 0.01f);
				// btn->showDown();
			} else {
				btn->callAfterDelay([btn] { btn->showUp(); }, 0.01f);
				// btn->showUp();
			}
		}
	});

	if (auto scrollBar = mPrimaryLayout->getSprite<ds::ui::ScrollBar>("scroll_bar")) {
		scrollBar->linkScrollList(fileList);
		scrollBar->getNubSprite()->setCornerRadius(0);
		scrollBar->getBackgroundSprite()->setCornerRadius(0);
	}

	if (auto entryField = mPrimaryLayout->getSprite<ds::ui::EntryField>("entry_field")) {
		auto entryText = entryField->getTextSprite();
		entryText->setFont("viewer:body");
		auto font_size = mEngine.getWafflesSettings().getFloat("search:list:item:font_size", 0, 20.0f);
		entryText->setFontSize(font_size);
		entryText->setColor(ci::ColorA::black());
		entryField->setColor(ci::ColorA::white());
		if (auto keyboard = mPrimaryLayout->getSprite<ds::ui::SoftKeyboard>("primary_keyboard")) {
			auto& setty			= keyboard->getSoftKeyboardSettings();
			setty.mKeyDownColor = ci::Color(mEngine.getColors().getColorFromName("waffles:button:bg:high:dark"));
			setty.mKeyUpColor	= ci::Color(mEngine.getColors().getColorFromName("waffles:button:bg:normal:dark"));
			setty.mGraphicType	= ds::ui::SoftKeyboardSettings::kSolid;
			setty.mGraphicRoundedCornerRadius = 0;
			keyboard->setSoftKeyboardSettings(setty);
			for (auto button : keyboard->getButtonVector()) {}
		}
		const auto maxChars = mEngine.getWafflesSettings().getInt("search:max_characters", 0, 50);
		entryField->setTextUpdatedCallback([this, entryField, maxChars](const std::wstring& text) {
			if (text.length() > maxChars) {
				// Trim the text back to the limit for the next frame
				// Needs to be after delay to avoid infinite text-upated loop
				entryField->callAfterDelay(
					[texty = text, entryField, maxChars] { entryField->setCurrentText(texty.substr(0, maxChars)); },
					0.01f);
			} else {
				startSearch(ds::utf8_from_wstr(text));
			}
			auto def = mPrimaryLayout->getSprite("entry_field_default");
			if (text.length() == 0) {
				def->show();
			} else {
				def->hide();
			}
		});
	}

	setupFilterButton("filter_bar.folder.the_button", "", 0);
	setupFilterButton("filter_bar.presentations.the_button", "record", 0);
	setupFilterButton("filter_bar.videos.the_button", "media", ds::Resource::VIDEO_TYPE);
	setupFilterButton("filter_bar.images.the_button", "media", ds::Resource::IMAGE_TYPE);
	setupFilterButton("filter_bar.pdfs.the_button", "media", ds::Resource::PDF_TYPE);
	setupFilterButton("filter_bar.links.the_button", "media", ds::Resource::WEB_TYPE);

	mPrimaryLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});

	mPrimaryLayout->runLayout();

	const float startWidth	= mPrimaryLayout->getWidth();
	const float startHeight = mPrimaryLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

	callAfterDelay([this] { startSearch(""); }, mEngine.getAnimDur());

	// setAnimateOnScript(mEngine.getWafflesSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));


	// these are to hide this from showing up in saved drawings
	mEventClient.listenToEvents<RequestPreDrawingSave>([this](auto& e) { hide(); });
	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) { show(); });
}


void SearchViewer::setupFilterButton(const std::string& buttonName, const std::string& mediaType,
									 const int resourceFilter) {
	if (!mPrimaryLayout) return;
	auto theButton = mPrimaryLayout->getSprite<ds::ui::LayoutButton>(buttonName);
	if (theButton) {
		theButton->setClickFn(
			[this, mediaType, theButton, resourceFilter] { handleFilterButton(theButton, mediaType, resourceFilter); });

		mFilterButtons.push_back(theButton);
	}
}

void SearchViewer::handleFilterButton(ds::ui::LayoutButton* butt, std::string mediaType, const int resourceFilter) {
	for (auto it : mFilterButtons) {
		it->showUp();
	}

	if (!butt) return;

	if (butt == mSelectedFilterButton) {
		mSelectedFilterButton = nullptr;
		mCurrentFilter		  = "";
		mResourceFilter		  = 0;
	} else {
		mSelectedFilterButton = butt;
		mSelectedFilterButton->showDown();
		mCurrentFilter	= mediaType;
		mResourceFilter = resourceFilter;
	}

	startSearch(mSearchText);
}

void SearchViewer::startSearch(const std::string& searchStr) {
	mSearchText = searchStr;
	mSearchQuery.start(
		[this, searchStr](SearchQuery& q) { q.setInput(mEngine, searchStr, mCurrentFilter, mResourceFilter); });
}

void SearchViewer::onSearchResults(SearchQuery& q) {
	if (!mPrimaryLayout) return;
	auto fileList = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) return;

	auto rootContent = mEngine.mContent.getChildByName("cms_root");

	if (auto bb = mPrimaryLayout->getSprite("back_button.the_button")) {
		bb->hide();
	}

	fileList->setContentList(q.mOutput);
	layout();
}

void SearchViewer::listFolder(ds::model::ContentModelRef theFolder) {
	if (!mPrimaryLayout) return;
	auto fileList = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) return;

	std::vector<ds::model::ContentModelRef> theContent;
	for (auto it : theFolder.getChildren()) {
		theContent.emplace_back(it);
	}


	if (auto bb = mPrimaryLayout->getSprite("back_button.the_button")) {
		bb->show();
	}

	int parentId = theFolder.getPropertyInt("parent_id");
	mPrimaryLayout->setSpriteClickFn("back_button.the_button", [this, parentId] {
		auto parentContent = mEngine.mContent.getChildByName("cms_root");
		if (parentId < 1) {
			listFolder(parentContent);
		} else {
			listFolder(parentContent.getDescendant("waffles_nodes", parentId));
		}
	});

	mPrimaryLayout->setSpriteText("back_button.normal_label", theFolder.getPropertyString("name"));
	mPrimaryLayout->setSpriteText("back_button.high_label", theFolder.getPropertyString("name"));

	fileList->setContentList(theContent);
	layout();
}

void SearchViewer::onLayout() {
	if (mPrimaryLayout) {
		mPrimaryLayout->runLayout();


		if (auto fileList = mPrimaryLayout->getSprite<ds::ui::SmartScrollList>("scroll_list")) {

			fileList->forEachLoadedSprite([this, fileList](ds::ui::Sprite* bs) {
				ds::ui::SmartLayout* rpi = dynamic_cast<ds::ui::SmartLayout*>(bs);
				if (rpi) {
					rpi->setSize(fileList->getWidth(), rpi->getHeight());
					rpi->runLayout();
				}
			});
		}
	}
}


} // namespace waffles
