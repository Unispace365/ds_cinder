#include "sprite_creator.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/touch/drag_destination_info.h>
#include <ds/ui/button/sprite_button.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include <typeinfo>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include "ui/tree_item.h"

namespace layout_builder {

SpriteCreator::SpriteCreator(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
{
	setPosition(20.0f, 300.0f);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	setTransparent(false);
	setColor(ci::Color(0.5f, 0.5f, 0.5f));

	mLayout = new ds::ui::LayoutSprite(mEngine);
	addChildPtr(mLayout);
	mLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mLayout->setSpacing(2.0f);
	mLayout->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	mLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);

	createCreateButton("sprite");
	createCreateButton("image");
	createCreateButton("image_with_thumbnail");
	createCreateButton("image_button");
	createCreateButton("sprite_button");
	createCreateButton("text");
	createCreateButton("multiline_text");
	createCreateButton("gradient");
	createCreateButton("layout");
	createCreateButton("circle");
	createCreateButton("border");
	createCreateButton("scroll_list");
	createCreateButton("scroll_area");
	createCreateButton("scroll_bar");

	layout();
	animateOn();
}

void SpriteCreator::createCreateButton(std::string typeName){
	if(!mLayout) return;
	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(mLayout, ds::Environment::expand("%APP%/data/layouts/creator_button.xml"), spriteMap);
	ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(spriteMap["label_field"]);
	if(texty){
		texty->setText(typeName);
	}
	ds::ui::SpriteButton* sb = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["the_button"]);
	if(sb){
		sb->setClickFn([this, typeName]{
			mEngine.getNotifier().notify(CreateSpriteRequest(typeName));
		});
	}
}

void SpriteCreator::onAppEvent(const ds::Event& in_e){
// 	if(in_e.mWhat == InspectTreeRequest::WHAT()){
// 		const InspectTreeRequest& e((const InspectTreeRequest&)in_e);
// 		inspectTree(e.mTree);
// 	} else if(in_e.mWhat == RefreshLayoutRequest::WHAT() || in_e.mWhat == LoadLayoutRequest::WHAT()){
// 		//clearTree();
// 	} else if(in_e.mWhat == InspectSpriteRequest::WHAT()){
// 		const InspectSpriteRequest& e((const InspectSpriteRequest&)in_e);
// 
// 		for(auto it = mTreeItems.begin(); it < mTreeItems.end(); ++it){
// 			(*it)->getValueField()->setColor(ci::Color::white());
// 			if((*it)->getLinkedSprite() == e.mSprid){
// 				(*it)->getValueField()->setColor(ci::Color(0.8f, 0.4f, 0.4f));
// 			}
// 		}
// 	}

}



void SpriteCreator::layout(){
	if(mLayout){
		mLayout->runLayout();
		setSize(mLayout->getWidth(), mLayout->getHeight() + 30.0f);
	}
}


void SpriteCreator::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void SpriteCreator::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder
