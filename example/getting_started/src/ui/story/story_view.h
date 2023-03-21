#pragma once
#ifndef _GETTING_STARTED_APP_UI_STORY_STORY_VIEW_H_
#define _GETTING_STARTED_APP_UI_STORY_STORY_VIEW_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
 * \class downstream::StoryView
 *			A view that shows a single story. Disappears when idle starts, and reappears on user action
 */
class StoryView : public ds::ui::SmartLayout {
  public:
	StoryView(ds::ui::SpriteEngine& eng);

  private:
	void animateOn();
	void animateOff();

	void setData();
};

} // namespace downstream

#endif
