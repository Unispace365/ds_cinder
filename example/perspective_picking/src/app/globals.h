#ifndef _PERSPECTIVEPICKING_APP_GLOBALS_
#define _PERSPECTIVEPICKING_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "model/all_data.h"

namespace ds { namespace ui {
	class SpriteEngine;
}} // namespace ds::ui

namespace perspective_picking {

/**
 * \class perspective_picking::Globals
 * \brief Global data for the app.
 */
class Globals {
  public:
	Globals(ds::ui::SpriteEngine&, const AllData& d);

	ds::ui::SpriteEngine& mEngine;
	ds::EventNotifier	  mNotifier;

	const AllStories& mAllStories;

	// Shortcuts
	const ds::cfg::Text& getText(const std::string& name) const;
	ds::cfg::Settings&	 getSettingsLayout() const;
	ds::cfg::Settings&	 getSettings(const std::string& name) const;
};

} // namespace perspective_picking

#endif // !_PERSPECTIVEPICKING_APP_GLOBALS_
