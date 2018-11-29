#include "stdafx.h"

#include "clock_and_weather_app.h"

#include <ds/app/engine/engine.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_viewer.h>

#include <cinder/app/RendererGl.h>

#include "events/app_events.h"
#include "ui/story/story_view.h"
#include "ui/weather/weather_view.h"

namespace downstream {

clock_and_weather_app::clock_and_weather_app()
	: ds::App()
	, mEventClient(mEngine)
	, mWeatherService(mEngine)
{

	// Register events so they can be called by string
	// after this registration, you can call the event like the following, or from an interface xml file
	// mEngine.getNotifier().notify("StoryDataUpdatedEvent");
	ds::event::Registry::get().addEventCreator(SomethingHappenedEvent::NAME(), [this]()->ds::Event* {return new SomethingHappenedEvent(); });

	mEventClient.listenToEvents<SomethingHappenedEvent>([this](const SomethingHappenedEvent& e) { std::cout << "Something happened" << std::endl; });

	//registerKeyPress("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); }, ci::app::KeyEvent::KEY_n);
}

void clock_and_weather_app::setupServer(){
	// add sprites
	mEngine.getRootSprite().addChildPtr(new StoryView(mEngine));

	/// NOTE: you'll need to add your own API key in app_settings.xml
	auto queryString = mEngine.getAppSettings().getString("weather:query", 0, "zip=97209,us");
	auto apiKey = mEngine.getAppSettings().getString("weather:api_key", 0, "");
	auto unity = mEngine.getAppSettings().getString("weather:unit", 0, "metric"); 
	auto timey = mEngine.getAppSettings().getDouble("weather:refresh_time", 0, 60);
	mWeatherService.initialize(queryString, apiKey, unity, timey);

	// For this test app, we show the app to start with for simplicity
	// In a real scenario, you'll probably want to start idled / attracting
	mEngine.stopIdling();
}

void clock_and_weather_app::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace downstream

// This line tells Cinder to actually create the application
CINDER_APP(downstream::clock_and_weather_app, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })
