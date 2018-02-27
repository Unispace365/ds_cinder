#include "stdafx.h"

#include "story_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>

#include "app/globals.h"
#include "events/app_events.h"


namespace fullstarter {

StoryView::StoryView(Globals& g)
	: ds::ui::SmartLayout(g.mEngine, "story_view.xml")
	, mGlobals(g)
{

	setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	hide();
	setOpacity(0.0f);
		
	listenToEvents<ds::app::IdleStartedEvent>([this](const ds::app::IdleStartedEvent& e) { animateOff(); });
	listenToEvents<ds::app::IdleEndedEvent>([this](const ds::app::IdleEndedEvent& e) { animateOn(); });
	listenToEvents<StoryDataUpdatedEvent>([this](const StoryDataUpdatedEvent& e) { setData(); });

}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data
	if(!mGlobals.mAllData.mStories.empty()){

		auto storyRef = mGlobals.mAllData.mStories.front();
		setSpriteText("message", storyRef.getTitle());
		
		setSpriteImage("primary_image", storyRef.getPrimaryResource());
		
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

void StoryView::onUpdateServer(const ds::UpdateParams& p){
	// any changes for this frame happen here
}



} // namespace fullstarter
