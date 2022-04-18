#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace fullstarter {

/**
* \class fullstarter::StoryView
*			A view that shows a single story. Disappears when idle starts, and reappears on user action
*/
class StoryView : public ds::ui::SmartLayout  {
public:
	StoryView(ds::ui::SpriteEngine& eng);
};

} // namespace fullstarter
