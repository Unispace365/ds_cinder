#ifndef _SYNC_VIDEO_PLAYER_APP_GLOBALS_
#define _SYNC_VIDEO_PLAYER_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {
class SpriteEngine;
} // namespace ui
} // namespace ds

namespace ds {

/**
 * \class ds::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine&			mEngine;

	//Shortcuts
	const ds::cfg::Text&			getText(const std::string& name) const;
	ds::cfg::Settings&				getSettingsLayout() const;
	ds::cfg::Settings&				getSettings(const std::string& name) const;


};

} // !namespace ds

#endif // !_SYNC_VIDEO_PLAYER_APP_GLOBALS_

