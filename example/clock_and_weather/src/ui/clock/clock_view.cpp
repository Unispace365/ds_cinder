#include "stdafx.h"

#include "clock_view.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/LocalDateTime.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "events/app_events.h"

namespace {
struct Init {
  public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("clock_view", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
				return new downstream::ClockView(enginey);
			});
		});
	}
};

Init INIT;
} // namespace


namespace downstream {

ClockView::ClockView(ds::ui::SpriteEngine& eng)
  : ds::ui::SmartLayout(eng, "clock_view.xml") {
	updateClock();
}


void ClockView::updateClock() {
	callAfterDelay([this] { updateClock(); }, 0.1);


	Poco::LocalDateTime ldt;
	auto				theD = Poco::DateTimeFormatter::format(
		   ldt, mEngine.getAppSettings().getString("clock:date_format", 0, "%d-%m-%Y %H:%M"));
	auto thet = Poco::DateTimeFormatter::format(
		ldt, mEngine.getAppSettings().getString("clock:time_format", 0, "%d-%m-%Y %H:%M"));
	if (thet.find("0") == 0) {
		thet = thet.substr(1);
	}
	setSpriteText("date", theD);
	setSpriteText("time", thet);
}

} // namespace downstream
