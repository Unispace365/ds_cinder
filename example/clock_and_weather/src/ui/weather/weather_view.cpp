#include "stdafx.h"

#include "weather_view.h"

#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/service/weather_service.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"

namespace {
class Init {
  public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("weather_view", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
				return new downstream::WeatherView(enginey);
			});

			/*
			e.registerSpritePropertySetter(
				"media_player_src", [](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string&
			fileReferrer) { if(auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) { ds::Resource
			theResource; int			 mediaType = ds::Resource::parseTypeFromFilename(theValue); if(mediaType ==
			ds::Resource::WEB_TYPE) { theResource = ds::Resource(theValue, mediaType); } else { std::string absPath =
			ds::filePathRelativeTo(fileReferrer, theValue); theResource = ds::Resource(absPath);
					}
					mediaPlayer->loadMedia(theResource, true);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_src on a non-mediaPlayer sprite");
					return;
				}
			});
			*/
		});
	}
};

Init INIT;
} // namespace


namespace downstream {

WeatherView::WeatherView(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "weather_view.xml") {

	listenToEvents<ds::weather::WeatherUpdatedEvent>([this](auto e) {
		completeAllTweens(false, true);
		setContentModel(mEngine.mContent.getChildByName("portland_weather"));
	});

	setContentModel(mEngine.mContent.getChildByName("portland_weather"));
}


} // namespace downstream
