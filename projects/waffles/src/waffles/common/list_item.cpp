#include "stdafx.h"

#include "list_item.h"

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

#include <ds/ui/interface_xml/interface_xml_importer.h>

#include "app/waffles_app_defs.h"

namespace waffles {

ListItem::ListItem(ds::ui::SpriteEngine& g)
	: ListItem::ListItem(g, "waffles/common/list_item.xml") {}

ListItem::ListItem(ds::ui::SpriteEngine& g, std::string layout_file)
	: ds::ui::SmartLayout(g, layout_file) {

	setContentUpdatedCallback([this] {
		bool validy = true;

		auto icony	= getSprite<ds::ui::Image>("icon");
		auto arrowy = getSprite("the_arrow");

		if (icony) {
			std::string thumbPath = "";
			auto		theType	  = getContentModel().getPropertyString("type_key");
			auto		mediaType = getContentModel().getPropertyResource("media").getType();

			if (arrowy) {
				arrowy->hide();
			}

			if (theType == "assets_folder" || theType == "playlist_folder" || theType == waffles::MEDIA_TYPE_DIRECTORY_CMS) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Folder_64.png";
				if (arrowy) {
					arrowy->show();
				}
			} else if (theType == "current_playlist") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Playlist_64.png";
				if (arrowy) {
					arrowy->show();
				}
			} else if (theType == "ambient") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Ambient_64.png";
			} else if (theType == "presentation") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Presentations_64.png";
				if (arrowy) {
					arrowy->show();
				}
			} else if (theType == "pinboard") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Pin_64.png";
				/* if (arrowy) {
					arrowy->show();
				} */
			} else if (theType == "browser") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Browser_64.png";
				/* if (arrowy) {
					arrowy->show();
				} */
			} else if (theType == "streams") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Stream_64.png";
				/* if (arrowy) {
					arrowy->show();
				} */
			} else if (theType == "search") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Search_64.png";
				/* if (arrowy) {
					arrowy->show();
				} */
			} else if (theType == "custom_layout_template" || theType == "slide-grid" || theType == "slide") {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Slide_64.png";
			} else if (theType == "media") {
				/* //thumbPath = "%APP%/data/images/waffles/icons/1x/Image_64.png";
			} else if (mediaType == ds::Resource::PDF_TYPE) {
				//thumbPath = "%APP%/data/images/waffles/icons/1x/PDF_64.png";
			} else if (mediaType == ds::Resource::VIDEO_TYPE || mediaType == ds::Resource::YOUTUBE_TYPE) {
				//thumbPath = "%APP%/data/images/waffles/icons/1x/Video_64.png";
			} else if (mediaType == ds::Resource::WEB_TYPE) {
				//thumbPath = "%APP%/data/images/waffles/icons/1x/Link_64.png"; */
			} else if (theType == "media" && mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Stream_64.png";
			} else {
				thumbPath = "%APP%/data/images/waffles/icons/1x/Local_Files_64.png";
				validy	  = true;
			}

			if (thumbPath.empty()) {
				icony->hide();

				if (auto tc = getSprite("thumbnail_clipper")) tc->show();
			} else {
				icony->setImageFile(thumbPath, ds::ui::Image::IMG_CACHE_F);
				if (auto tc = getSprite("thumbnail_clipper")) tc->hide();
			}
		}

		if (auto label = getSprite<ds::ui::Text>("name")) {
			std::string titleName = getContentModel().getPropertyString("record_name");

			if (!mSearchTerm.empty()) {
				std::string uppername = titleName;
				std::string upperserc = mSearchTerm;
				std::transform(uppername.begin(), uppername.end(), uppername.begin(), ::toupper);
				std::transform(upperserc.begin(), upperserc.end(), upperserc.begin(), ::toupper);
				auto findy = uppername.find(upperserc);
				if (findy != std::wstring::npos) {
					std::stringstream wss;
					std::string		  notoBoldName = mEngine.getSettings("FONTS").getString("noto-bold", 0, "");
					if (!notoBoldName.empty()) {
						wss << titleName.substr(0, findy) << L"<span font='" << notoBoldName << "'>"
							<< titleName.substr(findy, mSearchTerm.size()) << L"</span>"
							<< titleName.substr(findy + mSearchTerm.size());
					} else {
						wss << titleName;
					}
					titleName = wss.str();
				}
			}
			label->setText(titleName);
		}

		if(arrowy->visible() && getContentModel().getChildren().empty()){
			arrowy->enable(false);
			arrowy->setOpacity(0.4f);
		}

		if (validy) {
			enable(true);
			setOpacity(1.0f);
		} else {
			// enable(false);
			setOpacity(0.4f);
		}

		if (getContentModel().getPropertyResource("media_res").getType() == ds::Resource::PDF_TYPE ||
			getContentModel().getPropertyResource("media_media_res").getType() == ds::Resource::PDF_TYPE) {
			if (auto tc = getSprite("thumbnail_clipper")) {
				tc->setColor(ci::Color::white());
				tc->setTransparent(false);
			}
		}

		setState(0);

		completeAllTweens(false, true);
		setSize(getWidth(), getHeight());
		runLayout();
	});
}

void ListItem::animateOn(const float delay) {
	tweenAnimateOn(true, delay, 0.05f);
}

void ListItem::setState(const int buttonState) {

	auto background = getSprite("background_highlight");
	auto label		= getSprite("name");
	auto icon		= getSprite("icon");

	if (!background || !label || !icon) return;

	// highlighted
	if (buttonState == 1) {
		ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_selected");
		ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_selected_background");
		background->setColorA(backgroundColor);
		label->setColorA(textColor);
		icon->setColorA(textColor);

		// un highlighted
	} else {
		ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_normal");
		ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_background");
		background->setColorA(backgroundColor);
		label->setColorA(textColor);
		icon->setColorA(textColor);
	}
}

} // namespace waffles
