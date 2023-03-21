#ifndef _PANGO_APP_GLOBALS_
#define _PANGO_APP_GLOBALS_

#include "model/all_data.h"

namespace ds { namespace ui {
	class SpriteEngine;
}} // namespace ds::ui

namespace pango {

/**
 * \class pango::Globals
 * \brief Global data for the app.
 */
class Globals {
  public:
	Globals(ds::ui::SpriteEngine&, const AllData& d);

	ds::ui::SpriteEngine& mEngine;

	const AllData& mAllData;
};

} // namespace pango

#endif // !_PANGO_APP_GLOBALS_
