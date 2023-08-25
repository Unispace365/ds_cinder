#ifndef _CLOCK_AND_WEATHER_APP_H_
#define _CLOCK_AND_WEATHER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/service/weather_service.h>

namespace downstream {
class AllData;

class clock_and_weather_app : public ds::App {
  public:
	clock_and_weather_app();

	void setupServer();

	virtual void fileDrop(ci::app::FileDropEvent event) override;

  private:
	// App events can be handled here
	ds::EventClient mEventClient;

	ds::weather::WeatherService mWeatherService;
};

} // namespace downstream

#endif // !_CLOCK_AND_WEATHER_APP_H_
