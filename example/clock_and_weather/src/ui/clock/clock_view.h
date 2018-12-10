#pragma once
#ifndef _CLOCK_AND_WEATHER_APP_UI_CLOCK_CLOCK_VIEW
#define _CLOCK_AND_WEATHER_APP_UI_CLOCK_CLOCK_VIEW

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
* \class downstream::Clock
*			A view that shows the ClockView view clock
*/
class ClockView : public ds::ui::SmartLayout {
public:
	ClockView(ds::ui::SpriteEngine& eng);

	/// repeatedly called after calling it once
	void updateClock();
};

} // namespace downstream

#endif

