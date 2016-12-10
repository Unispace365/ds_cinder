#include "sprite_property_item.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include <typeinfo>
#include <ds/ui/interface_xml/interface_xml_importer.h>

namespace layout_builder {


SpritePropertyItem::SpritePropertyItem(Globals& g, const std::wstring& labelOne, const std::wstring& labelTwo)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mNameText(nullptr)
	, mLabelTextTwo(nullptr)
{
	float paddin = 10.0f;
	mLayoutLPad = paddin / 2.0f;
	mLayoutRPad = paddin / 2.0f;
	float theWiddy = 0.0f;
	float theHiddy = 0.0f;
	if(!labelOne.empty()){
		mNameText = mGlobals.getText("layout_builder:tree:item").create(mEngine, this);
		mNameText->setText(labelOne);
		mNameText->setPosition(0.0f, paddin / 2.0f);
		theWiddy += mNameText->getWidth() + paddin / 2.0f;
		theHiddy = mNameText->getFontSize() + paddin;
	}

	mLabelTextTwo = mGlobals.getText("layout_builder:tree:item_two").create(mEngine, this);
	mLabelTextTwo->setText(labelTwo);
	mLabelTextTwo->setPosition(theWiddy, paddin / 2.0f);
	theWiddy += mLabelTextTwo->getWidth() + paddin / 2.0f;
	if(mLabelTextTwo->getFontSize() + paddin > theHiddy){
		theHiddy = mLabelTextTwo->getFontSize() + paddin;
	}

	setSize(theWiddy, theHiddy);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
		if(mValueTappedCallback){
			mValueTappedCallback(this);
		}
	});
}

void SpritePropertyItem::setValueTappedCallback(std::function<void(SpritePropertyItem*)> tappedCallback){
	mValueTappedCallback = tappedCallback;
}

void SpritePropertyItem::setValueText(const std::wstring& labelTwoText){
	if(mLabelTextTwo){
		mLabelTextTwo->setText(labelTwoText);
	}
}


} // namespace layout_builder
