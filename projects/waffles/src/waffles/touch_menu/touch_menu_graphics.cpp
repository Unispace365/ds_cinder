#include "stdafx.h"

#include "touch_menu_graphics.h"


#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

TouchMenuGraphics::TouchMenuGraphics(ds::ui::SpriteEngine& g)
	: ds::ui::Sprite(g) {


	setOpacity(0.0f);
	hide();
}

void TouchMenuGraphics::animateOn() {
	show();
	tweenOpacity(1.0f, mEngine.getAnimDur());
}

void TouchMenuGraphics::animateOff() {
	tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [this] { hide(); });
}

} // namespace waffles
