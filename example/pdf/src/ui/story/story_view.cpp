#include "stdafx.h"

#include "story_view.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"


namespace downstream {

StoryView::StoryView(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "story_view.xml") {
	/// link a button to an action in the app
	setSpriteClickFn("idle_button.the_button", [this] {
		if (mEngine.isIdling()) {
			userInputReceived();
		} else {
			mEngine.startIdling();
		}
	});

	// called when setContentModel() is called on SmartLayout
	setContentUpdatedCallback([this] {
		/// Most model application happens in the layout file with the "model" property
		/// You can manually apply properties to items
		setSpriteImage("primary_image", getContentModel().getPropertyResource("resourceid"));

		completeAllTweens(false, true);
		clearAnimateOnTargets(true);
		runLayout();
		tweenAnimateOn(true, 0.0f, 0.05f);
	});

	/// an event happened somewhere else in the app, respond to it
	listenToEvents<SomethingHappenedEvent>([this](const SomethingHappenedEvent& e) {
		setSpriteText("subtitle_message", "You hit the 'Something' button at " + std::to_string(e.mEventOrigin.x) +
											  ", " + std::to_string(e.mEventOrigin.y));
	});
}


} // namespace downstream
