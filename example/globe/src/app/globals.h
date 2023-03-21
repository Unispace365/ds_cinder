#ifndef _GLOBEEXAMPLE_APP_GLOBALS_
#define _GLOBEEXAMPLE_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

#include <model/location_model.h>

namespace ds { namespace ui {
	class SpriteEngine;
}} // namespace ds::ui

namespace globe_example {

/**
 * \class globe_example::Globals
 * \brief Global data for the app.
 */


class Globals {
  public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine& mEngine;

	ds::cfg::Settings& getSettingsLayout() const;

	void genDataModel();

	std::vector<ds::model::LocationRef> mLocations;
};

} // namespace globe_example

#endif // !_GLOBEEXAMPLE_APP_GLOBALS_