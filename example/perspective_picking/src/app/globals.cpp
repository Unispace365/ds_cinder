#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

namespace perspective_picking {

/**
 * \class perspective_picking::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e, const AllData& d)
  : mEngine(e)
  , mAllStories(d.mAllStories) {}

ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

ds::cfg::Settings& Globals::getSettingsLayout() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}


const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);
}


} // namespace perspective_picking
