#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

class PinboardView : public ds::ui::SmartLayout {
public:
	PinboardView(ds::ui::SpriteEngine& g);

	void calculatePinboard();
	void refreshContent();

	std::vector<ds::ui::SmartLayout*> mRows;
	std::vector<ds::ui::SmartLayout*> mItems;

};

}
