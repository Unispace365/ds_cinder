#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

namespace mv {

/**
 * \class mv::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e , const AllData& d )
		: mEngine(e)
		, mAllData(d)
		, mAnimDuration(0.2f)
{
}

const ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

const ds::cfg::Settings& Globals::getSettingsLayout() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}


const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);

}

void Globals::initialize() {
	mAnimDuration = getSettingsLayout().getFloat("animation:duration", 0, mAnimDuration);
}

const float Globals::getAnimDur() {
	return mAnimDuration;
}



} // !namespace mv


