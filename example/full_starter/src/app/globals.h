#ifndef _FULLSTARTER_APP_GLOBALS_
#define _FULLSTARTER_APP_GLOBALS_

#include <ds/ui/sprite/sprite_engine.h>

#include "model/all_data.h"

namespace fullstarter {

/**
 * \class fullstarter::Globals
 * \brief Global data for the app.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&, const AllData& d);

	ds::ui::SpriteEngine&			mEngine;

	const AllData&					mAllData;

};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_GLOBALS_