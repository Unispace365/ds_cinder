#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

namespace globe_example {

/**
 * \class globe_example::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e )
		: mEngine(e)
{
}

const ds::cfg::Settings& Globals::getSettingsLayout() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}





} // !namespace globe_example
