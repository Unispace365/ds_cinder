#pragma once

#include <ds/ui/sprite/sprite.h>

namespace waffles {

/**
 * \class waffles::TouchMenuGraphics
 *			Graphics when you activated the touch menu
 */
class TouchMenuGraphics final : public ds::ui::Sprite {
  public:
	TouchMenuGraphics(ds::ui::SpriteEngine& g);

	void animateOn();
	void animateOff();

};

} // namespace waffles
