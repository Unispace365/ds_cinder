#include "ds/app/engine/engine_cfg.h"

#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"

namespace ds {

namespace {
const ds::cfg::Settings		EMPTY_SETTINGS;
}

/**
 * ds::EngineCfg
 */
EngineCfg::EngineCfg()
{
}

const ds::cfg::Settings& EngineCfg::getSettings(const std::string& name) const
{
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty name"));
		return EMPTY_SETTINGS;
	}
	if (mSettings.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty mSettings"));
		return EMPTY_SETTINGS;
	}
	auto it = mSettings.find(name);
	if (it == mSettings.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() settings does not exist"));
		return EMPTY_SETTINGS;
	}
	return it->second;
}

void EngineCfg::loadSettings(const std::string& name, const std::string& filename)
{
	ds::cfg::Settings&	settings = mSettings[name];
	ds::Environment::loadSettings(filename, settings);
}

} // namespace ds
