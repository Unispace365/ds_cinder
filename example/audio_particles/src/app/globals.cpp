#include "stdafx.h"

#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

namespace mv {

/**
 * \class mv::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e)
  : mEngine(e)
  , mAnimDuration(0.2f)
  , mRequestId(0)
  , mVolume(0.0) {}

ds::cfg::Settings& Globals::getSettings(const std::string& name) const {
	return mEngine.getEngineCfg().getSettings(name);
}

ds::cfg::Settings& Globals::getAppSettings() const {
	return mEngine.getEngineCfg().getSettings("app_settings");
}

const ds::cfg::Text& Globals::getText(const std::string& name) const {
	return mEngine.getEngineCfg().getText(name);
}

void Globals::initialize() {
	mAnimDuration = getAppSettings().getFloat("animation:duration", 0, mAnimDuration);
}

const float Globals::getAnimDur() {
	return mAnimDuration;
}


} // namespace mv
