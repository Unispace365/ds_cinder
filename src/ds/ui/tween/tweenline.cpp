#include "stdafx.h"

#include "ds/ui/tween/tweenline.h"

#include "ds/ui/sprite/sprite.h"
#include <cinder/Timeline.h>

namespace ds { namespace ui {

	/**
	 * \class Tweenline
	 */
	Tweenline::Tweenline(cinder::Timeline& tl)
	  : mTimeline(tl) {}

	cinder::Timeline& Tweenline::getTimeline() {
		return mTimeline;
	}

}} // namespace ds::ui
