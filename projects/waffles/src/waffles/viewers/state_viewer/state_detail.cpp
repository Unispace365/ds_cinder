#include "stdafx.h"

#include "state_detail.h"

#include <Poco/DateTimeFormatter.h>
#include <poco/DateTime.h>
#include <poco/DateTimeParser.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "app/app_defs.h"
//#include "events/app_events.h"
#include "events/state_events.h"
#include "state_item.h"

namespace waffles {

StateDetail::StateDetail(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/state/state_detail.xml")
	, mEventClient(g) {


	setSpriteClickFn("save_state.the_button", [this] {
		auto entryField = getSprite<ds::ui::EntryField>("entry_field");
		if (entryField) {
			mEngine.getNotifier().notify(StateSaveRequest(ds::utf8_from_wstr(entryField->getCurrentText())));

			auto theStates = mEngine.mContent.getChildByName("states");
			for (auto it : theStates.getChildren()) {
				if (it.getPropertyWString("name") == entryField->getCurrentText()) {
					setContentModel(it);
					break;
				}
			}

			hideDetails();
		}
	});


	setSpriteClickFn("cancel_button.the_button", [this] { hideDetails(); });
	setSpriteClickFn("open_state_button.the_button", [this] {
		auto entryField = getSprite<ds::ui::EntryField>("entry_field");
		if (entryField) {
			mEngine.getNotifier().notify(StateLoadRequest(ds::utf8_from_wstr(entryField->getCurrentText())));
		}
	});

	runLayout();

	setContentUpdatedCallback([this] {
		// set stuff

		if (auto entryField = getSprite<ds::ui::EntryField>("entry_field")) {
			entryField->setCurrentText(getContentModel().getPropertyWString("name"));
		}

		if (auto date = getSprite<ds::ui::Text>("date")) {
			if (getContentModel().getPropertyBool("is_saved")) {
				auto		tzd		   = 0;
				std::string dateString = Poco::DateTimeFormatter::format(
					Poco::DateTimeParser::parse(getContentModel().getPropertyString("save_date"), tzd),
					"%H:%M:%S %W %e %B %Y");
				date->setText(dateString);
			} else {
				date->setText("Not yet saved");
			}
		}

		if (auto count = getSprite<ds::ui::Text>("count")) {

			std::stringstream countStream;
			int				  theCount = getContentModel().getPropertyInt("viewer_count");
			countStream << theCount << " Viewer";
			if (theCount != 1) {
				countStream << "s";
			}

			count->setText(countStream.str());
		}

		layout();
	});
}

void StateDetail::hideDetails() {
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [this] { hide(); });
}

// no size changes of this here
void StateDetail::layout() {
	runLayout();
}

void StateDetail::onSizeChanged() {
	layout();
}

} // namespace waffles
