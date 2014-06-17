#include "globals.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/sprite_engine.h>
#include "app/app_defs.h"

namespace na {

/**
 * \class na::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e, const NodeTree& t)
		: mEngine(e)
		, mNodeTree(t) {
}

const ds::cfg::Settings& Globals::getSettingsLayout() const {
	// SETTINGS_LAYOUT generally defined in app_defs, and is application dependent
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}

const ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);
}

const ds::cfg::NinePatch& Globals::getNinePatch(const std::string& name) const {
	return mEngine.getEngineCfg().getNinePatch(name);
}

} // na
