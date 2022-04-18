#include "stdafx.h"

#include "story_controller.h"
#include "story_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>

#include "events/app_events.h"


namespace ds {

StoryController::StoryController(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_controller.xml")
{

	/// When the whole app has gone idle (timeout configurable in engine.xml)
	listenToEvents<ds::app::IdleStartedEvent>([this](const ds::app::IdleStartedEvent& e) {
		removeCurrentStory();
	});

	/// When an interaction has occurred 
	listenToEvents<ds::app::IdleEndedEvent>([this](const ds::app::IdleEndedEvent& e) {
		setData();
	});

	// NOTE: this event gets called anytime ANY data has changed, so use with discretion
	// If content got updated in other parts of the app (or other data not even related to this app)
	// this will still be called, so be sure it only changes when needed
	listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) {
		setData();
	});

}

void StoryController::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	/// Uses the "model" property on any xml-loaded children to map to this data model
	auto storyRef = mEngine.mContent.getChildByName("sqlite.sample_data.sample_data");


	// only make a change if something has changed
	if(!mCurrentStory || (mCurrentStory && storyRef != mCurrentStory->getContentModel())) {
		removeCurrentStory();

		/// Create a new story
		mCurrentStory = new StoryView(mEngine);
		addChildPtr(mCurrentStory);
		/// Note that animations happen internally in Story view when setting the data
		mCurrentStory->setContentModel(storyRef);
	}


	/// if your app has multiple stories or multiple places a story can show up at the same time, you could:
	auto allStories = mEngine.mContent.getChildByName("sqlite.sample_data");
	for(auto it : allStories.getChildren()) {
		/// add a story view per child
	}
}

void StoryController::removeCurrentStory() {
	if(mCurrentStory) {
		/// The animate off script has to finish or this sprite will leak, so be careful not to animate that sprite again
		auto cs = mCurrentStory;
		cs->tweenAnimateOff(true, 0.0f, 0.02f, [cs] { cs->release(); });
		mCurrentStory = nullptr;
	}
}

} // namespace ds

