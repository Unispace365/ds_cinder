#include "stdafx.h"

#include "composite_slide.h"

#include <ds/ui/media/media_player.h>

#include "app/app_defs.h"
//#include "app/cms_definitions.hpp"
#include "waffles/waffles_events.h"

namespace waffles {

CompositeSlide::CompositeSlide(ds::ui::SpriteEngine& engine, TemplateDef& def, ds::model::ContentModelRef content)
	: TemplateBase(engine, def, content) {
}

float CompositeSlide::animateOn(float delay, std::function<void(void)> finishedCb) {
	// onSizeChanged();
	if (mAnimStartCb) mAnimStartCb();
	mEngine.getNotifier().notify(waffles::RequestViewerLaunchEvent(
		waffles::ViewerCreationArgs(mContentModel, waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER, getLocalCenterPosition())));
	return tweenAnimateOn(true, delay, 0.05f, finishedCb);
}


} // namespace waffles
