#pragma once
#ifndef _FULLSTARTER_APP_UI_STORY_STORY_VIEW_H_
#define _FULLSTARTER_APP_UI_STORY_STORY_VIEW_H_

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

#endif
