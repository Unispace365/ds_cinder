#ifndef _PDF_LEAK_TESTER_APP_GLOBALS_
#define _PDF_LEAK_TESTER_APP_GLOBALS_

#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>
namespace ds {
namespace ui {
class SpriteEngine;
} // namespace ui
} // namespace ds

namespace downstream {

/**
 * \class downstream::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine&			mEngine;

	const float						getAnimDur();

	void							initialize();

	//Shortcuts
	const ds::cfg::Text&			getText(const std::string& name) const;
	ds::cfg::Settings&				getAppSettings() const;
	ds::cfg::Settings&				getSettings(const std::string& name) const;

private:

	float							mAnimationDuration;
};

} // !namespace downstream

#endif // !_PDF_LEAK_TESTER_APP_GLOBALS_
