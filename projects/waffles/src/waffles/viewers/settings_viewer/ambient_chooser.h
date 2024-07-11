#pragma once

#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/text.h>

namespace waffles {

class AmbientChooser : public ds::ui::SmartLayout {
  public:
	AmbientChooser(ds::ui::SpriteEngine& g);

  private:
	void updateButtons();
};

} // namespace waffles
