#pragma once
#ifndef WEATHER_SERVICE_WEATHER_SERVICE
#define WEATHER_SERVICE_WEATHER_SERVICE

#include <ds/app/event_client.h>
#include <ds/app/event.h>
#include <ds/network/https_client.h>

namespace ds {

class WeatherCurrentUpdatedEvent : public RegisteredEvent<WeatherCurrentUpdatedEvent> {};
class WeatherForecastUpdatedEvent : public RegisteredEvent<WeatherForecastUpdatedEvent> {};

/**
* \class downstream::WeatherService
*					Connects to openweathermap for current and forecast data and dispatches events
*
* Saves to a ContentModelRef with this format:

weather
	(properties)
	city_id
	city_name
	city_lat
	city_lon
	city_country
	sun_rise
	sun_set
	updated_at 
	
	conditions (one child for current, multiple children for forecast)~
		temperature
		temperature_min
		temperature_max
		temperature_unit (kelvin, metric, farenheit)
		temperature_string (30°F, 30.2°C, 289.2°K)
		humidity (99)
		humidity_unit (%)
		pressure (985.17)
		pressure_unit (hPa)
		wind_speed (meters per second for metric, mph for imperial)
		wind_speed_unit (m/s for metric/standard, mph for imperial)
		wind_name (Light breeze)
		wind_dir (degrees)
		wind_code (SE, NE, NW, etc)
		wind_dir_name (SouthEase, etc)
		cloud_percent (90)
		cloud_name (overcast clouds)
		visibility_miles
		visibility_meters
		precip_mode (no, rain or snow)
		precip_value
		precip_unit (1h or 3h)
		weather_code ( https://openweathermap.org/weather-conditions )
		weather_name (moderate rain)
		weather_icon (10d)
		weather_image (list images, such as partly_cloudy)
		weather_image_full (the full relative path such as %APP%/data/images/weather/partly_cloudy.png)

		TODO: ability to query multiple cities or whatnot
*/
class WeatherService {
public:
	WeatherService(ds::ui::SpriteEngine&);
	~WeatherService();

	void									initialize(const std::string& query, const std::string& apiKey, const std::string& units = "metric", const double requeryTime = 60.0);

private:
	void									getWeather();
	ds::ui::SpriteEngine&					mEngine;
	size_t									mCallbackId;
	ds::EventClient							mEventClient;
	ds::net::HttpsRequest					mCurrentRequest;
	ds::net::HttpsRequest					mForecastRequest;

	std::string								mQuery;
	std::string								mApiKey;
	std::string								mUnit;

	void									mapWeatherImages(const int& weatherCode, const std::string& iconCode, std::string& outImage, std::wstring& outEmoji);

	void									addAttributeParam(ci::XmlTree& node, ds::model::ContentModelRef& model, std::string attrLabel, std::string propLabel);
};


} // namespace ds

#endif 
