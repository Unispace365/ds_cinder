#include "stdafx.h"

#include "pinboard_button.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"
//#include "app/helpers.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/model/viewer_creation_args.h"

namespace {
auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
		e.registerSpriteImporter("pinboard_button", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
			return new waffles::PinboardButton(enginey, "waffles/pinboard/pinboard_button.xml");
		});


		e.registerSpriteImporter("pinboard_button_menu", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
			return new waffles::PinboardButton(enginey, "waffles/pinboard/pinboard_button_menu.xml");
		});
	});
	return true;
}();
}

namespace waffles {

PinboardButton::PinboardButton(ds::ui::SpriteEngine& eng, std::string theLayout)
	: ds::ui::SmartLayout(eng, theLayout) {


	setSpriteClickFn("the_button", [this] { toggleOnPinboard(); });
	mHelper = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();

	// listenToEvents<ds::ContentUpdatedEvent>([this](auto& e) { callAfterDelay([this] { setPinnedStatus(); }, 0.1f);
	// });
	listenToEvents<ds::ScheduleUpdatedEvent>([this](auto& e) { callAfterDelay([this] { setPinnedStatus(); }, 0.1f); });


	setContentUpdatedCallback([this] { setPinnedStatus(); });
}


bool PinboardButton::itemIsOnPinboard(ds::ui::SpriteEngine& eng, ds::model::ContentModelRef model) {
	auto helper		 = ds::model::ContentHelperFactory::getDefault<WafflesHelper>();
	auto thePinboard = helper->getPinboard();
	for (auto item : thePinboard.getChildren()) {
		if (item.getPropertyResource("media").getAbsoluteFilePath() ==
			model.getPropertyResource("media").getAbsoluteFilePath()) {
			return true;
		}
	}
	return false;
}

void PinboardButton::setPinnedStatus() {
	auto filledSpr	= getSprite("icon_filled");
	auto unFilled	= getSprite("icon_normal");
	auto spinner	= getSprite("spinner");
	auto filledSprH = getSprite("icon_filled_high");
	auto unFilledH	= getSprite("icon_high");
	auto spinnerH	= getSprite("spinner_high");

	if (!filledSpr || !unFilled || !spinner || !filledSprH || !unFilledH || !spinnerH) return;

	bool canBePinned = true;
	auto model		 = getContentModel();
	if (getContentModel().getPropertyString("type_key") != "media" ||
		getContentModel().getPropertyResource("media").empty()) {
		canBePinned = false;
	}

	/* if (getContentModel().getPropertyString("kind") == "note") {
		canBePinned = true;
	} */

	if (canBePinned && !mHelper->getPinboard().empty()) {
		show();
	} else {
		hide();
	}

	if(!mAmSaving){
		if (itemIsOnPinboard(mEngine, getContentModel())) {
			filledSpr->show();
			filledSprH->show();
			unFilled->hide();
			unFilledH->hide();
			/* spinner->hide();
			spinnerH->hide(); */
		} else {
			filledSpr->hide();
			filledSprH->hide();
			unFilled->show();
			unFilledH->show();
			/* spinner->hide();
			spinnerH->hide(); */
		}
	}
}


void PinboardButton::toggleOnPinboard() {
	if (getContentModel().empty()) {
		DS_LOG_WARNING("PinboardButton tried to toggle pinboard on an empty content model! Set the content model for "
					   "your pinboard buttons!");
		return;
	}

	auto filledSpr	= getSprite("icon_filled");
	auto unFilled	= getSprite("icon_normal");
	auto spinner	= getSprite("spinner");
	auto filledSprH = getSprite("icon_filled_high");
	auto unFilledH	= getSprite("icon_high");
	auto spinnerH	= getSprite("spinner_high");

	if (filledSpr && unFilled && spinner && filledSprH && unFilledH && spinnerH) {
		spinner->show();
		spinnerH->show();
		unFilled->hide();
		unFilledH->hide();
		filledSpr->hide();
		filledSprH->hide();
	}

	bool isOnPinboard = itemIsOnPinboard(mEngine, getContentModel());

	// Manually insert until the CMS sync comes back
	ds::model::ContentModelRef thePinboard = mEngine.mContent.getChildByName("current_pinboard");
	auto					   theList	   = thePinboard.getReferences("pinboard_items");

	if (!isOnPinboard) {
		// Add to pinboard
		theList[getContentModel().getId()] = getContentModel();
	} else {
		// Remove from pinboard
		theList.erase(getContentModel().getId());
	}

	listenToEvents<PinboardSaveComplete>([this, isOnPinboard](const PinboardSaveComplete& e) {
		if (getContentModel() == e.mContentModel && mAmSaving) {
			auto filledSpr	= getSprite("icon_filled");
			auto unFilled	= getSprite("icon_normal");
			auto spinner	= getSprite("spinner");
			auto filledSprH = getSprite("icon_filled_high");
			auto unFilledH	= getSprite("icon_high");
			auto spinnerH	= getSprite("spinner_high");

			if (filledSpr && unFilled && spinner && filledSprH && unFilledH && spinnerH) {
				spinner->hide();
				spinnerH->hide();
				if (!isOnPinboard) {
					// just added
					filledSpr->show();
					filledSprH->show();
				} else {
					unFilled->show();
					unFilledH->show();
				}
			}

			mAmSaving = false;
			stopListeningToEvents<PinboardSaveComplete>();
		}
	});

	mEngine.getNotifier().notify(RequestPinboardSave(getContentModel(), !isOnPinboard));
	mAmSaving = true;
}


void PinboardButton::onUpdateServer(const ds::UpdateParams& p) {

	auto spinner = getSprite("spinner");
	if (spinner && spinner->visible()) {
		spinner->setRotation(spinner->getRotation().z + p.getDeltaTime() * 60.0f * 3.0f);
	}
}

} // namespace waffles
