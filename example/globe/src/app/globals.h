#ifndef _GLOBEEXAMPLE_APP_GLOBALS_
#define _GLOBEEXAMPLE_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/cfg/cfg_nine_patch.h>
#include <ds/ui/sprite/sprite_engine.h>


namespace ds {
namespace ui {
class SpriteEngine;
} // namespace ui
} // namespace ds

namespace globe_example {

/**
 * \class globe_example::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine&			mEngine;

	const ds::cfg::Settings&		getSettingsLayout() const;


};

} // !namespace globe_example

#endif // !_GLOBEEXAMPLE_APP_GLOBALS_