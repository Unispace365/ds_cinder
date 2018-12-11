#pragma once
#ifndef _CLOCK_AND_WEATHER_APP_UI_WEATHER_VIEW
#define _CLOCK_AND_WEATHER_APP_UI_WEATHER_VIEW

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
* \class downstream::WeatherView
*			A view that shows the weather
*/
class WeatherView : public ds::ui::SmartLayout {
public:
	WeatherView(ds::ui::SpriteEngine& eng);

};

} // namespace downstream

#endif

