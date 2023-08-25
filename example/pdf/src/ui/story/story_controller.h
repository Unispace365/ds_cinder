#pragma once
#ifndef _PDF_EXAMPLE_APP_UI_STORY_STORY_CONTROLLER_H_
#define _PDF_EXAMPLE_APP_UI_STORY_STORY_CONTROLLER_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

class StoryView;

/**
 * \class downstream::StoryController
 *			Controls and manages any stories onscreen. If you don't rename this class into something relevant I will
 *publicly shame you.
 */
class StoryController : public ds::ui::SmartLayout {
  public:
	StoryController(ds::ui::SpriteEngine& eng);


	void setData();
	void removeCurrentStory();

	StoryView* mCurrentStory = nullptr;
};

} // namespace downstream

#endif
