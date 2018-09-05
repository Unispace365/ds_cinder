#include "stdafx.h"

#include "conversion_view.h"

#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>

#include <ds/ui/button/sprite_button.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/control/control_check_box.h>

#include "events/app_events.h"

#include "conversion_item.h"


namespace downstream {

ConversionView::ConversionView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "conversion_view.xml")
{
	listenToEvents<RequestConvertFiles>([this](auto& e) {
		auto scroller = getSprite<ds::ui::ScrollArea>("the_scroller");
		auto hodler = getSprite<ds::ui::LayoutSprite>("rows_hodler");
		if(!hodler || !scroller) return;

		for (auto it : e.mPaths){
			auto theRow = new ConversionItem(mEngine, it);
			hodler->addChildPtr(theRow);
			mItems.emplace_back(theRow);
			theRow->setCloseCallback([this, theRow, hodler, scroller] {
				mItems.erase(std::find(mItems.begin(), mItems.end(), theRow));
				relayout();
			});
		}

		relayout();
	});

	auto convertButton = getSprite<ds::ui::SpriteButton>("convert_button.the_button");
	if(convertButton) {
		convertButton->setClickFn([this] {
			startConversion();
		});
	}

	auto widthField = getSprite<ds::ui::EntryField>("entry_field");
	if(widthField) {
		widthField->setCurrentText(L"iw:ih");
		mEngine.registerEntryField(widthField);
		widthField->setNativeKeyboardCallback([this](ci::app::KeyEvent& event)->bool {
			if(event.getCode() == ci::app::KeyEvent::KEY_RETURN) {
				startConversion();
				return true;
			}
			return false;
		});
	}
}

void ConversionView::startConversion() {

	auto typeCheckBox = getSprite<ds::ui::ControlCheckBox>("h264");
	bool transParent = false;
	if(typeCheckBox) {
		transParent = typeCheckBox->getCheckBoxValue();
	}
	auto widthField = getSprite<ds::ui::EntryField>("entry_field");
	std::string scaleValue = "iw:ih";
	if(widthField) {
		scaleValue = widthField->getCurrentTextString();
	}

	for(auto it : mItems) {
		it->setTransparentType(transParent);
		it->setScaleRestriction(scaleValue);
		it->startConversion();
	}

}

void ConversionView::relayout() {
	callAfterDelay([this] {
		auto scroller = getSprite<ds::ui::ScrollArea>("the_scroller");
		auto hodler = getSprite<ds::ui::LayoutSprite>("rows_hodler");
		if(!hodler || !scroller) return;
		completeAllTweens(false, true);
		clearAnimateOnTargets(true);
		runLayout();
		hodler->runLayout();
		scroller->recalculateSizes();
	}, 0.01f);
}

} // namespace downstream

