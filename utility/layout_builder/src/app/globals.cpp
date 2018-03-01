#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

namespace layout_builder {

/**
 * \class layout_builder::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e , const AllData& d )
		: mEngine(e)
		, mAllData(d)
		, mAnimationDuration(0.35f)
{
}

const float Globals::getAnimDur(){
	return mAnimationDuration;
}

void Globals::initialize(){
	mAnimationDuration = getSettingsLayout().getFloat("animation:duration", 0, mAnimationDuration);
}

ds::cfg::Settings& Globals::getSettings(const std::string& name){
	return mEngine.getEngineCfg().getSettings(name);
}

ds::cfg::Settings& Globals::getSettingsLayout(){
	return mEngine.getEngineCfg().getSettings(SETTINGS_LAYOUT);
}



} // !namespace layout_builder
