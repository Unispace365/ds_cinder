#ifndef _WEB_EXAMPLE_APP_GLOBALS_
#define _WEB_EXAMPLE_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds { namespace ui {
	class SpriteEngine;
}} // namespace ds::ui

namespace web_example {

/**
 * \class web_example::Globals
 * \brief Global data for the app.
 */
class Globals {
  public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine& mEngine;
	ds::EventNotifier	  mNotifier;


	// Shortcuts
	const ds::cfg::Text& getText(const std::string& name) const;
	ds::cfg::Settings&	 getSettingsLayout() const;
	ds::cfg::Settings&	 getSettings(const std::string& name) const;
};

} // namespace web_example

#endif // !_WEB_EXAMPLE_APP_GLOBALS_
