#pragma once
#ifndef DS_APP_ENGINE_ENGINECFG_H_
#define DS_APP_ENGINE_ENGINECFG_H_

#include <unordered_map>
#include "ds/config/settings.h"

namespace ds {

/**
 * \class ds::EngineCfg
 * \brief Store all the engine configuration info.
 */
class EngineCfg {
public:
	EngineCfg();

	// Answer the requested settings file. In debug mode, throw
	// if it doesn't exist. In release mode, just answer an empty one.
	const ds::cfg::Settings&		getSettings(const std::string& name) const;

	// Convenice to load a setting file into the mEngineCfg settings.
	// @param name is the name that the system will use to refer to the settings.
	// @param filename is the leaf path of the settings file (i.e. "data.xml").
	// It will be loaded from all appropriate locations.
	void							loadSettings(const std::string& name, const std::string& filename);

private:
	EngineCfg(const EngineCfg&);

	std::unordered_map<std::string, ds::cfg::Settings>
									mSettings;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINECFG_H_
