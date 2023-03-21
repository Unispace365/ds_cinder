#pragma once
#ifndef _PDF_EXAMPLE_APP_UI_STORY_STORY_VIEW_H_
#define _PDF_EXAMPLE_APP_UI_STORY_STORY_VIEW_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
 * \class downstream::StoryView
 *			A view that shows a single story. Disappears when idle starts, and reappears on user action
 */
class StoryView : public ds::ui::SmartLayout {
  public:
	StoryView(ds::ui::SpriteEngine& eng);
};

} // namespace downstream

#endif
