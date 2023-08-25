#include "stdafx.h"

#include "weather_service.h"

#include <cinder/Xml.h>
#include <ds/debug/logger.h>

namespace ds { namespace weather {

	WeatherService::WeatherService(ds::ui::SpriteEngine& eng)
	  : mEngine(eng)
	  , mEventClient(eng)
	  , mCurrentRequest(eng)
	  , mForecastRequest(eng) {

		/*
		ds::model::ContentModelRef currentWeather = ds::model::ContentModelRef("current_weather");
		mEngine.mContent.addChild(currentWeather);

		ds::model::ContentModelRef weatherForecast = ds::model::ContentModelRef("weather_forecast");
		mEngine.mContent.addChild(weatherForecast);

		mEventClient.listenToEvents<SendMqttMessageEvent>([this](const SendMqttMessageEvent& e) {
		mMqttWatcher.sendOutboundMessage(e.mMessage);
		});
		*/

		mCurrentRequest.setReplyFunction([this](const bool errored, const std::string& reply, long httpCode) {
			if (errored || httpCode != 200) {
				DS_LOG_WARNING("WeatherService current request: got an invalid response. httpCode="
							   << httpCode << " reply: " << reply);
			} else {

				// std::cout << "got a reply " << std::endl << reply << std::endl;

				try {
					auto currentWeather = ds::model::ContentModelRef(mSettings.mName);

					ds::model::ContentModelRef conditions = ds::model::ContentModelRef("conditions");

					ci::XmlTree basey	= ci::XmlTree(reply);
					ci::XmlTree rooty	= basey.getChild("current");
					ci::XmlTree city	= rooty.getChild("city");
					ci::XmlTree coord	= city.getChild("coord");
					ci::XmlTree country = city.getChild("country");
					ci::XmlTree sun		= city.getChild("sun");
					ci::XmlTree upda	= rooty.getChild("lastupdate");
					ci::XmlTree temp	= rooty.getChild("temperature");
					ci::XmlTree humi	= rooty.getChild("humidity");
					ci::XmlTree pres	= rooty.getChild("pressure");
					ci::XmlTree wind	= rooty.getChild("wind");
					ci::XmlTree wspe	= wind.getChild("speed");
					ci::XmlTree wdir	= wind.getChild("direction");
					ci::XmlTree clou	= rooty.getChild("clouds");
					ci::XmlTree visi	= rooty.getChild("visibility");
					ci::XmlTree prec	= rooty.getChild("precipitation");
					ci::XmlTree weat	= rooty.getChild("weather");


					addAttributeParam(city, currentWeather, "id", "city_id");
					addAttributeParam(city, currentWeather, "name", "city_name");
					addAttributeParam(coord, currentWeather, "lon", "city_lon");
					addAttributeParam(coord, currentWeather, "lat", "city_lat");
					currentWeather.setProperty("city_country", country.getValue<std::string>());
					addAttributeParam(sun, currentWeather, "rise", "sun_rise");
					addAttributeParam(sun, currentWeather, "set", "sun_set");
					addAttributeParam(upda, currentWeather, "value", "updated_at");

					addAttributeParam(upda, conditions, "value", "time_from");
					addAttributeParam(upda, conditions, "value", "time_to");

					addAttributeParam(temp, conditions, "value", "temperature");
					addAttributeParam(temp, conditions, "min", "temperature_min");
					addAttributeParam(temp, conditions, "max", "temperature_max");
					addAttributeParam(temp, conditions, "unit", "temperature_unit");

					std::wstring tempString	   = conditions.getPropertyWString("temperature");
					auto		 theAmount	   = conditions.getPropertyFloat("temperature");
					int			 roundedAmount = (int)roundf(theAmount);
					if (mSettings.mUnits == "imperial") {
						tempString = std::to_wstring(roundedAmount) + L"°F";
					} else if (mSettings.mUnits == "metric") {
						tempString = std::to_wstring(roundedAmount) + L"°C";
					} else {
						tempString += L"°K";
					}
					conditions.setProperty("temperature_string", tempString);


					addAttributeParam(humi, conditions, "value", "humidity");
					addAttributeParam(humi, conditions, "unit", "humidity_unit");

					addAttributeParam(pres, conditions, "value", "pressure");
					addAttributeParam(pres, conditions, "unit", "pressure_unit");

					addAttributeParam(wspe, conditions, "value", "wind_speed");
					addAttributeParam(wspe, conditions, "name", "wind_name");

					std::string windSpeedUnit = "m/s";
					if (mSettings.mUnits == "imperial") {
						windSpeedUnit = "mph";
					}
					conditions.setProperty("wind_speed_unit", windSpeedUnit);

					addAttributeParam(wdir, conditions, "value", "wind_dir");
					addAttributeParam(wdir, conditions, "code", "wind_code");
					addAttributeParam(wdir, conditions, "name", "wind_dir_name");

					addAttributeParam(clou, conditions, "value", "cloud_percent");
					addAttributeParam(clou, conditions, "name", "cloud_name");

					addAttributeParam(visi, conditions, "value", "visibility_meters");
					conditions.setProperty("visibility_miles",
										   (int)round(conditions.getPropertyDouble("visibility_meters") * 0.000621371));

					addAttributeParam(prec, conditions, "mode", "precip_mode");
					addAttributeParam(prec, conditions, "unit", "precip_unit");

					addAttributeParam(weat, conditions, "number", "weather_code");
					addAttributeParam(weat, conditions, "value", "weather_name");
					addAttributeParam(weat, conditions, "icon", "weather_icon");

					auto wCode = conditions.getPropertyInt("weather_code");
					auto wIcon = conditions.getPropertyString("weather_icon");

					std::wstring weatherEmoji = L"🌥";
					std::string	 iconPng	  = "";

					mapWeatherImages(wCode, wIcon, iconPng, weatherEmoji);

					conditions.setProperty("weather_emoji", weatherEmoji);
					conditions.setProperty("weather_image", iconPng);
					conditions.setProperty("weather_image_full", "%APP%/data/images/weather/" + iconPng + ".png");

					currentWeather.addChild(conditions);
					// currentWeather.printTree(true);

					bool									found = false;
					std::vector<ds::model::ContentModelRef> allChillins;
					for (auto it : mEngine.mContent.getChildren()) {
						if (it.getName() == mSettings.mName) {
							found = true;
							allChillins.emplace_back(currentWeather);
						} else {
							allChillins.emplace_back(it);
						}
					}

					if (!found) {
						allChillins.emplace_back(currentWeather);
					}

					mEngine.mContent.setChildren(allChillins);

					std::string forecastQuery = "http://api.openweathermap.org/data/2.5/forecast?" + mSettings.mQuery +
												"&APPID=" + mSettings.mApiKey + "&mode=xml&units=" + mSettings.mUnits;
					mForecastRequest.makeGetRequest(forecastQuery);

				} catch (std::exception& e) {
					DS_LOG_WARNING("WeatherService: current forecast error parsing xml: " << e.what());
				}
			}
		});

		mForecastRequest.setReplyFunction([this](const bool errored, const std::string& reply, long httpCode) {
			if (errored || httpCode != 200) {
				DS_LOG_WARNING("WeatherService forecast request: got an invalid response. httpCode="
							   << httpCode << " reply: " << reply);
			} else {

				ci::XmlTree basey	 = ci::XmlTree(reply);
				ci::XmlTree rooty	 = basey.getChild("weatherdata");
				ci::XmlTree forecast = rooty.getChild("forecast");

				auto&									baseThing = mEngine.mContent.getChildByName(mSettings.mName);
				std::vector<ds::model::ContentModelRef> childrens;
				std::vector<ds::model::ContentModelRef> conditions;
				childrens.emplace_back(baseThing.getChild(0));
				ds::model::ContentModelRef theForecast = ds::model::ContentModelRef("forecast");

				for (auto it = forecast.getChildren().begin(); it != forecast.getChildren().end(); ++it) {
					auto aForecast = parseForecastItem(*it->get());
					conditions.emplace_back(aForecast);
				}
				theForecast.setChildren(conditions);
				childrens.emplace_back(theForecast);
				baseThing.setChildren(childrens);
				// baseThing.printTree(true);

				mEngine.getNotifier().notify(WeatherUpdatedEvent());
			}
		});
	}

	ds::model::ContentModelRef WeatherService::parseForecastItem(ci::XmlTree item) {
		ds::model::ContentModelRef conditions = ds::model::ContentModelRef("forecast");

		addAttributeParam(item, conditions, "from", "time_from");
		addAttributeParam(item, conditions, "to", "time_to");

		ci::XmlTree weat = item.getChild("symbol");
		ci::XmlTree prec = item.getChild("precipitation");
		ci::XmlTree wdir = item.getChild("windDirection");
		ci::XmlTree wspe = item.getChild("windSpeed");
		ci::XmlTree temp = item.getChild("temperature");
		ci::XmlTree pres = item.getChild("pressure");
		ci::XmlTree humi = item.getChild("humidity");
		ci::XmlTree clou = item.getChild("clouds");

		addAttributeParam(temp, conditions, "value", "temperature");
		addAttributeParam(temp, conditions, "min", "temperature_min");
		addAttributeParam(temp, conditions, "max", "temperature_max");
		addAttributeParam(temp, conditions, "unit", "temperature_unit");

		std::wstring tempString	   = conditions.getPropertyWString("temperature");
		auto		 theAmount	   = conditions.getPropertyFloat("temperature");
		int			 roundedAmount = (int)roundf(theAmount);
		if (mSettings.mUnits == "imperial") {
			tempString = std::to_wstring(roundedAmount) + L"°F";
		} else if (mSettings.mUnits == "metric") {
			tempString = std::to_wstring(roundedAmount) + L"°C";
		} else {
			tempString += L"°K";
		}
		conditions.setProperty("temperature_string", tempString);


		addAttributeParam(humi, conditions, "value", "humidity");
		addAttributeParam(humi, conditions, "unit", "humidity_unit");

		addAttributeParam(pres, conditions, "value", "pressure");
		addAttributeParam(pres, conditions, "unit", "pressure_unit");

		addAttributeParam(wspe, conditions, "mps", "wind_speed");
		addAttributeParam(wspe, conditions, "name", "wind_name");

		std::string windSpeedUnit = "m/s";
		if (mSettings.mUnits == "imperial") {
			windSpeedUnit = "mph";
		}
		conditions.setProperty("wind_speed_unit", windSpeedUnit);

		addAttributeParam(wdir, conditions, "value", "wind_dir");
		addAttributeParam(wdir, conditions, "code", "wind_code");
		addAttributeParam(wdir, conditions, "name", "wind_dir_name");

		addAttributeParam(clou, conditions, "all", "cloud_percent");
		addAttributeParam(clou, conditions, "value", "cloud_name");

		// addAttributeParam(visi, conditions, "value", "visibility_meters");
		// conditions.setProperty("visibility_miles", (int)round(conditions.getPropertyDouble("visibility_meters") *
		// 0.000621371));

		addAttributeParam(prec, conditions, "type", "precip_mode");
		addAttributeParam(prec, conditions, "value", "precip_value_mm");
		conditions.setProperty("precip_value_in", conditions.getPropertyFloat("precip_value_mm") * 0.0393701f);
		addAttributeParam(prec, conditions, "unit", "precip_unit");

		addAttributeParam(weat, conditions, "number", "weather_code");
		addAttributeParam(weat, conditions, "name", "weather_name");
		addAttributeParam(weat, conditions, "var", "weather_icon");

		auto wCode = conditions.getPropertyInt("weather_code");
		auto wIcon = conditions.getPropertyString("weather_icon");

		std::wstring weatherEmoji = L"🌥";
		std::string	 iconPng	  = "";

		mapWeatherImages(wCode, wIcon, iconPng, weatherEmoji);

		conditions.setProperty("weather_emoji", weatherEmoji);
		conditions.setProperty("weather_image", iconPng);
		conditions.setProperty("weather_image_full", "%APP%/data/images/weather/" + iconPng + ".png");

		return conditions;
	}

	void WeatherService::mapWeatherImages(const int& wCode, const std::string& wIcon, std::string& iconPng,
										  std::wstring& weatherEmoji) {

		/// https://openweathermap.org/weather-conditions

		/* we assume you have these images (dupe some if you don't have the full set or something)
		clear_day.png				01d
		clear_night.png				01n
		cloudy.png					02d 02n
		foggy.png					50d
		foggy_night.png				50n
		freezing.png				13d			511
		heavy_drizzle.png						302, 312, 313, 314, 321
		heavy_rain.png							502, 503, 504
		heavy_storms.png						200-232, 771, 781
		light_drizzle.png						300, 301, 310, 311
		light_rain.png							500, 501, 520, 522, 531
		partly_cloudy_night.png		02n
		partly_sunny.png			02d
		snowing.png					13n
		stormy.png					11d 11n
		windy.png
		*/

		switch (wCode) {
		case 800:
			iconPng		 = "clear_day";
			weatherEmoji = L"☀";
			break;
		case 511:
			iconPng		 = "freezing";
			weatherEmoji = L"❄";
			break;
		case 302:
		case 312:
		case 313:
		case 314:
		case 321:
			iconPng		 = "heavy_drizzle";
			weatherEmoji = L"🌧";
			break;
		case 502:
		case 503:
		case 504:
			iconPng		 = "heavy_rain";
			weatherEmoji = L"🌧";
			break;
		case 200:
		case 201:
		case 202:
		case 210:
		case 211:
		case 212:
		case 221:
		case 230:
		case 231:
		case 232:
			iconPng		 = "heavy_storms";
			weatherEmoji = L"🌩";
			break;
		case 300:
		case 301:
		case 310:
		case 311:
			iconPng		 = "light_drizzle";
			weatherEmoji = L"🌧";
			break;
		case 500:
		case 501:
		case 520:
		case 521:
		case 522:
		case 531:
			iconPng		 = "light_rain";
			weatherEmoji = L"🌧";
			break;

		default:
			break;
		}

		if (iconPng.empty()) {
			if (wIcon == "01d") {
				iconPng		 = "clear_day";
				weatherEmoji = L"☀";
			} else if (wIcon == "01n") {
				iconPng		 = "clear_night";
				weatherEmoji = L"🌙";
			} else if (wIcon == "02d" || wIcon == "02n") {
				iconPng		 = "cloudy";
				weatherEmoji = L"☁️";
			} else if (wIcon == "50d") {
				iconPng		 = "foggy";
				weatherEmoji = L"🌫";
			} else if (wIcon == "50n") {
				iconPng		 = "foggy_night";
				weatherEmoji = L"🌫";
			} else if (wIcon == "13d") {
				iconPng		 = "freezing";
				weatherEmoji = L"❄";
			} else if (wIcon == "02n") {
				iconPng		 = "partly_cloudy_night";
				weatherEmoji = L"⛅";
			} else if (wIcon == "02d") {
				iconPng		 = "partly_sunny";
				weatherEmoji = L"⛅";
			} else if (wIcon == "13n") {
				iconPng		 = "snowing";
				weatherEmoji = L"🌨";
			} else if (wIcon == "11d" || wIcon == "11n") {
				iconPng		 = "stormy";
				weatherEmoji = L"🌩";
			} else if (wCode >= 200 && wCode < 300) {
				iconPng		 = "heavy_storms";
				weatherEmoji = L"🌩";
			} else if (wCode >= 300 && wCode < 600) {
				iconPng		 = "light_rain";
				weatherEmoji = L"🌧";
			} else if (wCode >= 600 && wCode < 700) {
				iconPng		 = "freezing";
				weatherEmoji = L"❄";
			} else if (wCode >= 700 && wCode < 800) {
				iconPng		 = "foggy";
				weatherEmoji = L"🌫";
			} else if (wCode > 800 && wCode < 900) {
				iconPng		 = "cloudy";
				weatherEmoji = L"☁️";
			}
		}

		if (iconPng.empty()) iconPng = "partly_sunny";
	}

	void WeatherService::addAttributeParam(ci::XmlTree& node, ds::model::ContentModelRef& model, std::string attrLabel,
										   std::string propLabel) {
		if (node.hasAttribute(attrLabel)) {
			model.setProperty(propLabel, node.getAttributeValue<std::string>(attrLabel));
		}
	}


	WeatherService::~WeatherService() {
		mEngine.cancelTimedCallback(mCallbackId);
	}

	void WeatherService::initialize(WeatherSettings settings) {
		mSettings = settings;
		mEngine.cancelTimedCallback(mCallbackId);
		getWeather();

		if (settings.mRequeryTime > 0) {
			mCallbackId = mEngine.repeatedCallback([this] { getWeather(); }, settings.mRequeryTime);
		}
	}

	void WeatherService::getWeather() {
		if (mSettings.mQuery.empty()) {
			DS_LOG_WARNING("WeatherService: need a query string for weather service");
		}

		if (mSettings.mApiKey.empty()) {
			DS_LOG_WARNING("WeatherService: need an api key for weather service. see https://openweathermap.org/api ");
		}

		std::string currentQuery = "http://api.openweathermap.org/data/2.5/weather?" + mSettings.mQuery +
								   "&APPID=" + mSettings.mApiKey + "&mode=xml&units=" + mSettings.mUnits;
		mCurrentRequest.makeGetRequest(currentQuery);
	}

}} // namespace ds::weather
