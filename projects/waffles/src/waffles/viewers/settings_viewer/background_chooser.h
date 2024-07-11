#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/sprite.h>

namespace waffles {

class BackgroundChooser : public ds::ui::SmartLayout {
  public:
	BackgroundChooser(ds::ui::SpriteEngine& g);

  private:
	std::function<void(void)> mButtonCallback;

	void updateButtons();
};

} // namespace waffles
