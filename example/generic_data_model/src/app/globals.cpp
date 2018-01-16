#include "stdafx.h"

#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "app_defs.h"

#include "query/data_wrangler.h"

namespace downstream {

/**
 * \class downstream::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e, DataWrangler& dataWrangler)
	: mEngine(e)
	, mAnimationDuration(0.35f)
	, mDataWrangler(dataWrangler)
{
}

const float Globals::getAnimDur(){
	return mAnimationDuration;
}

void Globals::initialize(){
	mAnimationDuration = getAppSettings().getFloat("animation:duration", 0, mAnimationDuration);
}

ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

ds::cfg::Settings& Globals::getAppSettings() const {
	return mEngine.getEngineCfg().getSettings(SETTINGS_APP);
}


const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);

}



} // !namespace downstream

