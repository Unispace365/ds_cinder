#include "tree_item.h"

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

TreeItem::TreeItem(Globals& g, ds::ui::Sprite* linkedItem)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mLinkedSprite(linkedItem)
	, mNameText(nullptr)
	, mLabelTextTwo(nullptr)
{
	initialize(mLinkedSprite->getSpriteName(), ds::wstr_from_utf8(typeid(*mLinkedSprite).name()));
	
}


TreeItem::TreeItem(Globals& g, const std::wstring& labelOne, const std::wstring& labelTwo)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mLinkedSprite(nullptr)
	, mNameText(nullptr)
	, mLabelTextTwo(nullptr)
{
	initialize(labelOne, labelTwo);

}

void TreeItem::initialize(const std::wstring& labelOne, const std::wstring& labelTwo){
	float paddin = 10.0f;
	mLayoutLPad = paddin / 2.0f;
	mLayoutRPad = paddin/2.0f;
	float theWiddy = 0.0f;
	float theHiddy = 0.0f;
	if(!labelOne.empty()){
		mNameText = mGlobals.getText("tree:item").create(mEngine, this);
		mNameText->setText(labelOne);
		mNameText->setPosition(0.0f, paddin/2.0f);
		theWiddy += mNameText->getWidth() + paddin/2.0f;
		theHiddy = mNameText->getFontSize() + paddin;
	}
	if(!labelTwo.empty()){
		mLabelTextTwo = mGlobals.getText("tree:item_two").create(mEngine, this);
		mLabelTextTwo->setText(labelTwo);
		mLabelTextTwo->setPosition(theWiddy, paddin/2.0f);
		theWiddy += mLabelTextTwo->getWidth() + paddin/2.0f;
		if(mLabelTextTwo->getFontSize() + paddin > theHiddy){
			theHiddy = mLabelTextTwo->getFontSize() + paddin;
		}
	}
	setSize(theWiddy, theHiddy);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
		if(mLinkedSprite){
			std::cout << ds::utf8_from_wstr(mLinkedSprite->getSpriteName()) << std::endl;
			mEngine.getNotifier().notify(InspectSpriteRequest(mLinkedSprite));
		} else {
			std::cout << "DO SOMETHING ALREADY!" << std::endl;
		}
	});
}


} // namespace layout_builder
