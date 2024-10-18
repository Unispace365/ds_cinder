#include "stdafx.h"

#include "composite_slide.h"

#include <ds/ui/media/media_player.h>

#include "app/waffles_app_defs.h"
//#include "app/cms_definitions.hpp"
#include "waffles/waffles_events.h"

namespace waffles {

CompositeSlide::CompositeSlide(ds::ui::SpriteEngine& engine, TemplateDef& def, ds::model::ContentModelRef content, std::string channel_name)
	: TemplateBase(engine, def, content) {
	
	if (!channel_name.empty()) {
		mEventClient.setNotifier(mEngine.getChannel(channel_name));
		mEventClient.start();

	} else {
		mEventClient.setNotifier(mEngine.getNotifier());
		mEventClient.start();
	}

}

float CompositeSlide::animateOn(float delay, std::function<void(void)> finishedCb) {
	// onSizeChanged();
	if (mAnimStartCb) mAnimStartCb();
	
	return tweenAnimateOn(true, delay, 0.05f, [this, finishedCb]() {
		mEventClient.notify(
			waffles::RequestViewerLaunchEvent(
				waffles::ViewerCreationArgs(
					mContentModel,
					waffles::VIEW_TYPE_TITLED_MEDIA_VIEWER,
					getLocalCenterPosition()
				)
			)
		);
		if(finishedCb) finishedCb();
		});
}

float CompositeSlide::animateOff(float delay, std::function<void(void)> finishedCb) {
	mEventClient.notify(waffles::RequestCloseAllEvent(mContentModel));
	return tweenAnimateOff(true, delay, 0.00f, finishedCb);
}

} // namespace waffles

