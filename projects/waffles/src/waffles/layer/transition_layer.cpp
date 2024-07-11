#include "stdafx.h"

#include "transition_layer.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/color_util.h>

//#include "events/app_events.h"
#include "waffles/template/template_base.h"
//#include <app/helpers.h>
#include "waffles/waffles_events.h"


namespace waffles {

TransitionLayer::TransitionLayer(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "waffles/layer/transition_layer.xml") {
	if (mEngine.getAppSettings().getBool("hpi:layer:transition:disable", 0, true)) {
		return;
	}
	// Handle background change requests
	listenToEvents<waffles::TransitionRequest>([this](const waffles::TransitionRequest& event) { startTransition(); });
}

void TransitionLayer::startTransition() {
	mEngine.getNotifier().notify(waffles::TransitionReady());
	mEngine.getNotifier().notify(waffles::TransitionComplete());
}

} // namespace waffles
