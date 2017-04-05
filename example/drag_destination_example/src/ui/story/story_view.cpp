#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/touch/drag_destination_info.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace example {

StoryView::StoryView(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mSourceText(nullptr)
{

	mSourceText = mGlobals.getText("sample:config").create(mEngine, this);
	if(mSourceText){
		// This is the sprite that can be dragged on to a target
		mSourceText->setText("Drag me"); 

		// The draggable sprite has got to be 
		mSourceText->enable(true);
		mSourceText->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
		mSourceText->setDragDestinationCallback([this](ds::ui::Sprite* destinationSprite, const ds::ui::DragDestinationInfo& di){
			if(di.mPhase == ds::ui::DragDestinationInfo::Entered){
				mDestinationText->setText("Drag entered");
			} else if(di.mPhase == ds::ui::DragDestinationInfo::Exited){
				mDestinationText->setText("Drag target");
			} else if(di.mPhase == ds::ui::DragDestinationInfo::Released){
				mDestinationText->setText("Drag released");
			}
		});
	}

	mDestinationText = mGlobals.getText("sample:config").create(mEngine, this);
	if(mDestinationText){
		// This is the sprite that is the target area. (The drop area of the drag and drop)
		// This doesn't need to be a text sprite, it can be any kind of sprite. 
		// We're using text here so we can clearly update the text with what's going on
		mDestinationText->setText("Drag target");

		// Any sprites that are a drag destination target need to be added to the drag destination list
		mEngine.addToDragDestinationList(mDestinationText);
	}

	// calls layout
	setData();
	animateOn();

}

void StoryView::onAppEvent(const ds::Event& in_e){

}

void StoryView::setData() {


	layout();
}

void StoryView::layout(){
	if(mSourceText){
		mSourceText->setPosition(100.0f, 200.0f);
	}

	if(mDestinationText){
		mDestinationText->setPosition(200.0f, 400.0f);
	}
}


void StoryView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f));
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}

} // namespace example
