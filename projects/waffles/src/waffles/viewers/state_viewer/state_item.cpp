#include "stdafx.h"

#include "state_item.h"

#include <Poco/DateTimeFormatter.h>
#include <poco/DateTime.h>
#include <poco/DateTimeParser.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/sprite/sprite_engine.h>

#include <ds/ui/interface_xml/interface_xml_importer.h>

#include "app/app_defs.h"
#include "events/state_events.h"

namespace waffles {
StateItem::StateItem(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/state/state_item.xml") {
	setSpriteClickFn("delete_state_button.the_button", [this] {
		mEngine.getNotifier().notify(StateDeleteRequest(getContentModel().getPropertyString("name")));
	});
	setSpriteClickFn("open_state_button.the_button", [this] {
		mEngine.getNotifier().notify(StateLoadRequest(getContentModel().getPropertyString("name")));
	});

	setContentUpdatedCallback([this] {
		bool validy = true;

		/*
		if(mThumbnail){
			std::string thumbPath = mInfoModel.getPrimaryResource().getThumbnailFilePath();

			if(thumbPath.empty()){
				mThumbnail->clearImage();
			} else {
				mThumbnail->setImageFile(thumbPath, ds::ui::Image::IMG_CACHE_F);
			}
		}
			*/


		if (auto date = getSprite<ds::ui::Text>("date")) {
			auto		tzd		   = 0;
			std::string dateString = Poco::DateTimeFormatter::format(
				Poco::DateTimeParser::parse(getContentModel().getPropertyString("save_date"), tzd),
				"%H:%M:%S %W %e %B %Y");
			date->setText(dateString);
		}

		if (auto count = getSprite<ds::ui::Text>("count")) {
			std::wstringstream countStream;
			int				   theCount = getContentModel().getPropertyInt("viewer_count");
			countStream << theCount << L" Viewer";
			if (theCount != 1) {
				countStream << "s";
			}

			count->setText(countStream.str());
		}

		if (validy) {
			enable(true);
			setOpacity(1.0f);
		} else {
			enable(false);
			setOpacity(0.8f);
		}

		setState(0);
		layout();
	});
}


void StateItem::animateOn(const float delay) {
	tweenAnimateOn(true, delay, 0.05f);
}

void StateItem::layout() {
	completeAllTweens(false, true);
	clearAnimateOnTargets(true);
	// setSize(getWidth(), getHeight());
	runLayout();
}


void StateItem::setState(const bool highlighted) {

	auto background = getSprite("background");
	auto date		= getSprite("date");
	auto count		= getSprite("count");
	auto title		= getSprite("title");
	if (!background || !date || !count || !title) return;

	if (highlighted) {
		ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_selected");
		ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_selected_background");
		background->setColorA(backgroundColor);
		title->setColorA(textColor);
		date->setColorA(textColor);
		count->setColorA(textColor);
	} else {
		ci::ColorA textColor	   = mEngine.getColors().getColorFromName("ui_normal");
		ci::ColorA backgroundColor = mEngine.getColors().getColorFromName("ui_background");
		background->setColorA(backgroundColor);
		title->setColorA(textColor);
		date->setColorA(textColor);
		count->setColorA(textColor);
	}
}

} // namespace waffles
