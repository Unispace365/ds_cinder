#pragma once
#ifndef _CLOCK_AND_WEATHER_APP_UI_BACKGROUND_BACKGROUND_VIEW
#define _CLOCK_AND_WEATHER_APP_UI_BACKGROUND_BACKGROUND_VIEW

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
 * \class downstream::BackgroundView
 *			A view that shows the background
 */
class BackgroundView : public ds::ui::SmartLayout {
  public:
	BackgroundView(ds::ui::SpriteEngine& eng);

	void cycleBackground();

	std::vector<std::string> mBackgrounds;
	ds::ui::SmartLayout*	 mCurBackground;

	int mCurBackgroundIndex = 0;
};

} // namespace downstream

#endif
