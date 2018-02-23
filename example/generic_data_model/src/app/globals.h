#ifndef _GENERIC_DATA_MODEL_APP_GLOBALS_
#define _GENERIC_DATA_MODEL_APP_GLOBALS_

#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {
class SpriteEngine;
} // namespace ui
} // namespace ds

namespace downstream {
class DataWrangler;

/**
 * \class downstream::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&, DataWrangler& dataWrangler);

	ds::ui::SpriteEngine&			mEngine;

	DataWrangler&					mDataWrangler;

	void							initialize();

};

} // !namespace downstream

#endif // !_GENERIC_DATA_MODEL_APP_GLOBALS_
