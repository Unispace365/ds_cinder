#include "stdafx.h"

#include "story_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/service/weather_service.h>

#include "events/app_events.h"


namespace downstream {

StoryView::StoryView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_view.xml")
{

	listenToEvents<ds::weather::WeatherUpdatedEvent>([this](auto e) {
		callAfterDelay([this] {
			completeAllTweens(false, true);
			clearAnimateOnTargets(true);
			runLayout();
		//	tweenAnimateOn(true, 0.0f, 0.05f);
		}, 0.01f);
	});
}

} // namespace downstream

