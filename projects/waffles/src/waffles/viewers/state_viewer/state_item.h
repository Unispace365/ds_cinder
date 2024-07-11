#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

class StateItem : public ds::ui::SmartLayout {
  public:
	StateItem(ds::ui::SpriteEngine& g);

	void animateOn(const float delay);

	void layout();

	void setState(const bool highlighted);
};

} // namespace waffles
