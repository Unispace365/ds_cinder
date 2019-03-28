#pragma once
#ifndef _FULLSTARTER_APP_UI_STORY_STORY_CONTROLLER_H_
#define _FULLSTARTER_APP_UI_STORY_STORY_CONTROLLER_H_

#include <ds/ui/layout/smart_layout.h>

namespace fullstarter {

class StoryView;

/**
* \class fullstarter::StoryController
*			Controls and manages any stories onscreen. If you don't rename this class into something relevant I will publicly shame you.
*/
class StoryController : public ds::ui::SmartLayout {
public:
	StoryController(ds::ui::SpriteEngine& eng);


	void setData();
	void removeCurrentStory();

	StoryView*	mCurrentStory = nullptr;
};

} // namespace fullstarter

#endif
