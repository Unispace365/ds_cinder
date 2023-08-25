#include "stdafx.h"

#include "text_resize_controller.h"
#include "text_resize_view.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"


namespace text_resize {

TextResizeController::TextResizeController(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "text_resize_controller.xml") {
	mResizeView = new TextResizeView(mEngine);
	addChildPtr(mResizeView);

	// testing controls
}

} // namespace text_resize
