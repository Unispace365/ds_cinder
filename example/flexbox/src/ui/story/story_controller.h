#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace ds {

class StoryView;

/**
* \class ds::StoryController
*			Controls and manages any stories onscreen. If you don't rename this class into something relevant I will publicly shame you.
*/
class StoryController : public ds::ui::SmartLayout {
public:
	StoryController(ds::ui::SpriteEngine& eng);


	void setData();
	void removeCurrentStory();

	StoryView*	mCurrentStory = nullptr;
};

} // namespace ds


