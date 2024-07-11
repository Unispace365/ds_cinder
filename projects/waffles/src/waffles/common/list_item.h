#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

class ListItem : public ds::ui::SmartLayout {
  public:
	ListItem(ds::ui::SpriteEngine& g);

	ListItem(ds::ui::SpriteEngine& g, std::string layout_file);

	void setSearchTerm(const std::string& searchTerm) { mSearchTerm = searchTerm; }

	void animateOn(const float delay);

	// Highlighted or not
	// 1 == highlighted, 0 = normal
	void setState(const int buttonState);

	std::string mSearchTerm;
};

} // namespace waffles
