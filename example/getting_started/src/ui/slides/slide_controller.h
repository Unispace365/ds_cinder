#pragma once
#ifndef _GETTING_STARTED_APP_UI_SLIDES_SLIDE_CONTROLLER
#define _GETTING_STARTED_APP_UI_SLIDES_SLIDE_CONTROLLER

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
 * \class downstream::SlideController
 *			Load and display slides
 */
class SlideController : public ds::ui::SmartLayout {
  public:
	SlideController(ds::ui::SpriteEngine& eng);

	void setData(const bool doAnimate, const bool forwards = true);
	void goForward();
	void goBack();

  private:
	ds::ui::SmartLayout* mCurrentSlide;
};

} // namespace downstream

#endif

#pragma once
