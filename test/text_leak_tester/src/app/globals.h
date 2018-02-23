#ifndef _TEXT_LEAK_TESTER_APP_GLOBALS_
#define _TEXT_LEAK_TESTER_APP_GLOBALS_

#include <ds/ui/sprite/sprite_engine.h>

#include "model/all_data.h"

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
	Globals(ds::ui::SpriteEngine&, const AllData& d);

	ds::ui::SpriteEngine&			mEngine;

	const AllData&					mAllData;

};

} // !namespace downstream

#endif // !_TEXT_LEAK_TESTER_APP_GLOBALS_
