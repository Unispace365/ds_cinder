#include "stdafx.h"

#include "launcher_search_content.h"

#include <ds/app/environment.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {

LauncherSearchContent::LauncherSearchContent(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/launcher/launcher_search_content.xml")
	, mSearchQuery(g, []() { return new SearchQuery(); }) {

	mSearchQuery.setReplyHandler([this](SearchQuery& q) { onSearchResults(q); });

	const float itemSize = mEngine.getAppSettings().getFloat("info_list:item:height", 0, 100.0f);

	auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) {
		DS_LOG_WARNING("No file scroll list found for search viewer");
		return;
	}

	fileList->setContentItemTappedCallback([this](ds::ui::SmartLayout* sl, ds::model::ContentModelRef theModel) {
		if (sl) {
			const int itemId	 = sl->getContentModel().getId();
			auto	  theContent = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", itemId);
			auto	  theType	 = theContent.getPropertyString("type");

			if (theType == MEDIA_TYPE_DIRECTORY_CMS) {
				listFolder(theModel);
			} else {
				auto vca = ViewerCreationArgs(theContent, VIEW_TYPE_TITLED_MEDIA_VIEWER, getGlobalCenterPosition());
				/* DS_LOG_INFO("Trying to open viewer at " << vca.mLocation);
				// No auto-fullscreen for you!
				vca.mFromCenter = true;
				vca.mFullscreen = false;
				vca.mCheckBounds = false; */
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
			}
		}
	});

	fileList->setContentItemUpdatedCallback([this, fileList](ds::ui::SmartLayout* sl) {
		auto cm = sl->getContentModel();

		std::string titleName = cm.getPropertyString("name");
		if (!mSearchText.empty()) {
			std::string uppername = titleName;
			std::string upperserc = mSearchText;
			std::transform(uppername.begin(), uppername.end(), uppername.begin(), ::toupper);
			std::transform(upperserc.begin(), upperserc.end(), upperserc.begin(), ::toupper);
			auto findy = uppername.find(upperserc);
			if (findy != std::string::npos) {
				std::stringstream wss;
				std::string notoBoldName = mEngine.getAppSettings().getString("search:bold_font", 0, "BrownPro Bold");
				if (!notoBoldName.empty()) {
					wss << titleName.substr(0, findy) << "<span font='" << notoBoldName << "'>"
						<< titleName.substr(findy, mSearchText.size()) << "</span>"
						<< titleName.substr(findy + mSearchText.size());
				} else {
					wss << titleName;
				}
				titleName = wss.str();
			}
		}
		sl->setSpriteText("title", titleName);
		sl->setSpriteText("title_high", titleName);

		sl->setSize(fileList->getWidth(), sl->getHeight());

		auto icon = sl->getSprite<ds::ui::Image>("icon");
		if (icon) {
			std::string thumbPath = "";
			auto		theType	  = sl->getContentModel().getPropertyString("type");
			auto		mediaType = sl->getContentModel().getPropertyResource("media_res").getType();

			
			if (theType == MEDIA_TYPE_DIRECTORY_CMS || theType == MEDIA_TYPE_DIRECTORY_LOCAL) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Folder_64.png";
			} else if (theType == MEDIA_TYPE_PRESENTATION) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Presentations_64.png";
			} else if (theType == MEDIA_TYPE_PRESET) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Slide_64.png";
			} else if (mediaType == ds::Resource::IMAGE_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Image_64.png";
			} else if (mediaType == ds::Resource::PDF_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/PDF_64.png";
			} else if (mediaType == ds::Resource::VIDEO_TYPE || mediaType == ds::Resource::YOUTUBE_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Video_64.png";
			} else if (mediaType == ds::Resource::WEB_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Link_64.png";
			} else if (mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Stream_64.png";
			} else {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Folder_64.png";
			}

			icon->setImageFile(thumbPath, ds::ui::Image::IMG_CACHE_F);
		}
		sl->runLayout();
	});

	fileList->setStateChangeCallback([this](ds::ui::Sprite* sp, const bool highlighted) {
		auto theSl = dynamic_cast<ds::ui::SmartLayout*>(sp);
		if (!theSl) return;
		auto background = theSl->getSprite("background_highlight");
		auto label		= theSl->getSprite("name");
		auto icon		= theSl->getSprite<ds::ui::Image>("icon");

		if (!background || !label || !icon) return;


		if (highlighted) {
			ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_selected");
			ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_selected_background");
			background->setColorA(backgroundColor);
			label->setColorA(textColor);
			icon->setColorA(textColor);

		} else {
			ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_normal");
			ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_background");
			background->setColorA(backgroundColor);
			label->setColorA(textColor);
			icon->setColorA(textColor);
		}
	});

	fileList->setLayoutParams(0.0f, 0.0f, itemSize + 2.0f);

	auto scrollBar = getSprite<ds::ui::ScrollBar>("scroll_bar");
	if (scrollBar) {
		scrollBar->linkScrollList(fileList);
		ci::ColorA nubbinColor = mEngine.getColors().getColorFromName("ui_highlight");
		scrollBar->getNubSprite()->setColor(nubbinColor);
		scrollBar->getBackgroundSprite()->setOpacity(0.05f);
	}


	if (auto entryField = getSprite<ds::ui::EntryField>("entry_field")) {
		const auto maxChars = mEngine.getAppSettings().getInt("search:max_characters", 0, 50);
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
		});
	}

	setupFilterButton("filter_bar.folder.the_button", MEDIA_TYPE_DIRECTORY_CMS, 0);
	setupFilterButton("filter_bar.presentations.the_button", MEDIA_TYPE_PRESENTATION, 0);
	setupFilterButton("filter_bar.videos.the_button", MEDIA_TYPE_FILE_CMS, ds::Resource::VIDEO_TYPE);
	setupFilterButton("filter_bar.images.the_button", MEDIA_TYPE_FILE_CMS, ds::Resource::IMAGE_TYPE);
	setupFilterButton("filter_bar.pdfs.the_button", MEDIA_TYPE_FILE_CMS, ds::Resource::PDF_TYPE);
	setupFilterButton("filter_bar.links.the_button", MEDIA_TYPE_FILE_CMS, ds::Resource::WEB_TYPE);

	runLayout();

	const float startWidth	= getWidth();
	const float startHeight = getHeight() + 400.f;

	setSize(startWidth, startHeight);

	callAfterDelay([this] { startSearch(""); }, mEngine.getAnimDur());

	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void LauncherSearchContent::setupFilterButton(const std::string& buttonName, const std::string& mediaType,
											  const int resourceFilter) {
	auto theButton = getSprite<ds::ui::LayoutButton>(buttonName);
	if (theButton) {
		theButton->setClickFn(
			[this, mediaType, theButton, resourceFilter] { handleFilterButton(theButton, mediaType, resourceFilter); });

		mFilterButtons.push_back(theButton);
	}
}

void LauncherSearchContent::handleFilterButton(ds::ui::LayoutButton* butt, std::string mediaType,
											   const int resourceFilter) {
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

void LauncherSearchContent::startSearch(const std::string& searchStr) {
	mSearchText = searchStr;
	// mSearchQuery.start([this, searchStr](SearchQuery& q) { q.setInput(mEngine, searchStr, mResourceFilter); });
}

void LauncherSearchContent::onSearchResults(SearchQuery& q) {
	auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) return;

	auto rootContent = mEngine.mContent.getChildByName("cms_root");

	std::vector<ds::model::ContentModelRef> theContent;
	for (auto it : q.mOutput) {
		auto activeNodes = mEngine.mContent.getChildByName("cms_current_events").getPropertyListInt("active_nodes");

		if (std::find(activeNodes.begin(), activeNodes.end(), it.getId()) != activeNodes.end()) {
			auto theThingy = rootContent.getDescendant("waffles_nodes", it.getId());
			if (theThingy) {
				if (mCurrentFilter.empty()) {
					theContent.emplace_back(theThingy);
				} else if (theThingy.getPropertyString("type") == mCurrentFilter) {
					if (mCurrentFilter == MEDIA_TYPE_FILE_CMS) {
						auto theType = theThingy.getPropertyResource("media_res").getType();
						if (theType == mResourceFilter ||
							(theType == ds::Resource::YOUTUBE_TYPE && mResourceFilter == ds::Resource::VIDEO_TYPE)) {
							theContent.emplace_back(theThingy);
						}
					} else {
						theContent.emplace_back(theThingy);
					}
				}
			}
		}
	}

	if (auto bb = getSprite("back_button.the_button")) {
		bb->hide();
	}

	fileList->setContentList(theContent);
	runLayout();
}

void LauncherSearchContent::listFolder(ds::model::ContentModelRef theFolder) {
	auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) return;

	std::vector<ds::model::ContentModelRef> theContent;
	for (auto it : theFolder.getChildren()) {
		theContent.emplace_back(it);
	}


	if (auto bb = getSprite("back_button.the_button")) {
		bb->show();
	}

	int parentId = theFolder.getPropertyInt("parent_id");
	setSpriteClickFn("back_button.the_button", [this, parentId] {
		auto parentContent = mEngine.mContent.getChildByName("cms_root");
		if (parentId < 1) {
			listFolder(parentContent);
		} else {
			listFolder(parentContent.getDescendant("waffles_nodes", parentId));
		}
	});

	setSpriteText("back_button.normal_label", theFolder.getPropertyString("name"));
	setSpriteText("back_button.high_label", theFolder.getPropertyString("name"));

	fileList->setContentList(theContent);
	runLayout();
}

void LauncherSearchContent::onLayout() {

	if (auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list")) {

		fileList->forEachLoadedSprite([fileList](ds::ui::Sprite* bs) {
			ds::ui::SmartLayout* rpi = dynamic_cast<ds::ui::SmartLayout*>(bs);
			if (rpi) {
				rpi->setSize(fileList->getWidth(), rpi->getHeight());
				rpi->runLayout();
			}
		});
	}
}

} // namespace waffles
