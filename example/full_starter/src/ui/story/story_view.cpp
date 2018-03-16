#include "stdafx.h"

#include "story_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>

#include "events/app_events.h"


namespace fullstarter {

StoryView::StoryView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_view.xml")
{

	hide();
	setOpacity(0.0f);
		
	listenToEvents<ds::app::IdleStartedEvent>([this](const ds::app::IdleStartedEvent& e) { animateOff(); });
	listenToEvents<ds::app::IdleEndedEvent>([this](const ds::app::IdleEndedEvent& e) { animateOn(); });
	listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) { setData(); });
	listenToEvents<SomethingHappenedEvent>([this](const SomethingHappenedEvent& e) { 
		setSpriteText("subtitle_message", "You hit the 'Something' button at " + std::to_string(e.mEventOrigin.x) + ", " + std::to_string(e.mEventOrigin.y)); 
	});

}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data
	if(!mEngine.mContent.getChildren().empty()){

		/// Uses the "model" property on any xml-loaded children to map to this data model
		auto storyRef = mEngine.mContent.getChildByName("sqlite.sample_data.sample_data");
		setContentModel(storyRef);
		
		/// You can also manually apply properties to items
		setSpriteImage("primary_image", storyRef.getPropertyResource("resourceid"));
		
	}

	completeAllTweens(false, true);
	clearAnimateOnTargets(true);
	runLayout();
	tweenAnimateOn(true, 0.0f, 0.05f);
}


void StoryView::animateOn(){
	show();
	tweenOpacity(1.0f, mEngine.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace fullstarter
