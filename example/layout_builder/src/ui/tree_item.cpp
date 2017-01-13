#include "tree_item.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/button/image_button.h>

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
	
	ds::ui::ImageButton* deleteButton = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/close_button.png", "%APP%/data/images/ui/close_button.png", 10.0f);
	deleteButton->setScale(0.25f, 0.25f);
	deleteButton->getHighImage().setColor(ci::Color(0.7f, 0.2f, 0.2f));
	addChildPtr(deleteButton);
	deleteButton->setClickFn([this]{
		callAfterDelay([this]{
			mEngine.getNotifier().notify(DeleteSpriteRequest(mLinkedSprite));
		}, 0.01f);
	});


	float paddin = 10.0f;
	mLayoutLPad = paddin / 2.0f;
	mLayoutRPad = paddin/2.0f;
	float theWiddy = 0.0f;
	float theHiddy = 0.0f;
	mNameText = mGlobals.getText("tree:item").create(mEngine, this);
	mNameText->setText(mLinkedSprite->getSpriteName());
	mNameText->setPosition(deleteButton->getScaleWidth(), paddin / 2.0f);
	theWiddy += mNameText->getWidth() + paddin / 2.0f + deleteButton->getScaleWidth();
	theHiddy = mNameText->getFontSize() + paddin;
	

	mLabelTextTwo = mGlobals.getText("tree:item_two").create(mEngine, this);
	mLabelTextTwo->setText(ds::ui::XmlImporter::getSpriteTypeForSprite(mLinkedSprite));
	mLabelTextTwo->setPosition(theWiddy, paddin / 2.0f);
	theWiddy += mLabelTextTwo->getWidth() + paddin / 2.0f;
	if(mLabelTextTwo->getFontSize() + paddin > theHiddy){
		theHiddy = mLabelTextTwo->getFontSize() + paddin;
	}

	setTransparent(false);
	setColor(ci::Color(0.1f, 0.1f, 0.1f));
	setSize(theWiddy, theHiddy);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		if(mLinkedSprite){
			mEngine.getNotifier().notify(InspectSpriteRequest(mLinkedSprite));
		} 
	});
}

void TreeItem::setValueText(const std::wstring& labelTwoText){
	if(mLabelTextTwo){
		mLabelTextTwo->setText(labelTwoText);
	}
}


} // namespace layout_builder
