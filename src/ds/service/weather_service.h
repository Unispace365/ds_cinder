#pragma once
#ifndef WEATHER_SERVICE_WEATHER_SERVICE
#define WEATHER_SERVICE_WEATHER_SERVICE

#include <ds/app/event_client.h>
#include <ds/app/event.h>
#include <ds/network/https_client.h>

namespace ds {
namespace weather {

class WeatherUpdatedEvent : public RegisteredEvent<WeatherUpdatedEvent> {};



struct WeatherSettings {
	WeatherSettings() {};
	WeatherSettings(const std::string& name, const std::string& query, const std::string& apiKey, const std::string& units = "metric", const double requeryTime = 60.0)
		: mName(name), mQuery(query), mApiKey(apiKey), mUnits(units), mRequeryTime(requeryTime) {}

	/// The name of this weather result, which will be the name of the ContentModel child on mEngine.mContent
	std::string mName;

	/// A query string for the location, such as q=London,uk or zip=97202,us or lat=123&lon=45 or id=12345 (see openweathermap for id's)
	std::string mQuery;

	/// API key for openweathermap.org
	std::string mApiKey;

	/// metric (celcius), imperial (farenheit), or standard (kelvin)
	std::string mUnits;

	/// How often to re-query the weather in seconds. Set to 0 for only 1 response
	/// Recommend a minimum of 60 seconds, the underlying weather only updates once an hour, so more often is useless
	double mRequeryTime;
};
/**
* \class downstream::WeatherService
*					Connects to openweathermap for current and forecast data and dispatches events
*
* Saves to a ContentModelRef delivered to mEngine.mContent as a child with a name set from the settings with this format:

weather - name from WeatherSettings
	(properties)
	city_id
	city_name
	city_lat
	city_lon
	city_country
	sun_rise
	sun_set
	updated_at

	conditions (current conditions, always the first child) and forecast (all other children)
		time_from
		time_to
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
		precip_value_mm (in mm)
		precip_value_in (inches)
		precip_unit (1h or 3h)
		weather_code ( https://openweathermap.org/weather-conditions )
		weather_name (moderate rain)
		weather_icon (10d, see url above for list)
		weather_image (plain image name without an extension, see list below, such as partly_cloudy)
		weather_image_full (the full relative path such as %APP%/data/images/weather/partly_cloudy.png)
		weather_emoji (a string with a single emoji for weather code)


Image list:
clear_day
clear_night
cloudy
foggy
foggy_night
freezing
heavy_drizzle
heavy_rain
heavy_storms
light_drizzle
light_rain
partly_cloudy_night
partly_sunny
snowing
stormy
windy (currently unused)
*/
class WeatherService {
public:
	WeatherService(ds::ui::SpriteEngine&);
	~WeatherService();

	void									initialize(WeatherSettings settings);

private:
	void									getWeather();
	ds::ui::SpriteEngine&					mEngine;
	size_t									mCallbackId;
	ds::EventClient							mEventClient;
	ds::net::HttpsRequest					mCurrentRequest;
	ds::net::HttpsRequest					mForecastRequest;

	WeatherSettings							mSettings;

	ds::model::ContentModelRef				parseForecastItem(ci::XmlTree item);
	void									mapWeatherImages(const int& weatherCode, const std::string& iconCode, std::string& outImage, std::wstring& outEmoji);
	void									addAttributeParam(ci::XmlTree& node, ds::model::ContentModelRef& model, std::string attrLabel, std::string propLabel);
};

} // namespace weather
} // namespace ds

#endif 
