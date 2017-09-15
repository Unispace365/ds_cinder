#pragma once
#ifndef DS_APP_ENGINE_ENGINECFG_H_
#define DS_APP_ENGINE_ENGINECFG_H_

#include <map>
#include <unordered_map>
#include "ds/cfg/cfg_text.h"
#include "ds/cfg/settings.h"

namespace ds {

/**
 * \class ds::EngineCfg
 * \brief Store all the engine configuration info.
 */
class EngineCfg {
public:
	EngineCfg(ds::cfg::Settings& engine_settings);

	// Answer the requested settings file. In debug mode, throw
	// if it doesn't exist. In release mode, just answer an empty one.
	ds::cfg::Settings&				getSettings(const std::string& name);

	// Get the settings in the list AFTER the one specified by the name
	ds::cfg::Settings&				getNextSettings(const std::string& name);

	// Answer the requested text cfg. 
	// if it doesn't exist. In release mode, just answer an empty one.
	bool							hasText(const std::string& name) const;
	const ds::cfg::Text&			getText(const std::string& name) const;
	const std::string&				getDefaultTextCfgName() const;
	const ds::cfg::Text&			getDefaultTextCfg() const;
	void							setText(const std::string& name, const ds::cfg::Text&);

	// Answers true if settings with given key is already loaded
	bool							hasSettings(const std::string& name) const;

	/** Convenience to load a setting file into the mEngineCfg settings.
		It will be loaded from all appropriate locations.
		\param name is the name that the system will use to refer to the settings.
		\param filename is the leaf path of the settings file (i.e. "data.xml"). */
	void							loadSettings(const std::string& name, const std::string& filename);

	/** Convenience to save a setting file from the mEngineCfg settings.
		It will be saved to the user setting location.
		\param name is the name that the system will use to refer to the settings.
		\param filename is the leaf path of the settings file (i.e. "data.xml"). */
	void							saveSettings(const std::string& name, const std::string& filename);

	/** Convenience to append a setting file into the existing mEngineCfg settings.
		It will NOT be loaded from all appropriate locations.
		\param name is the name that the system will use to refer to the settings.
		\param filename is the FULL path of the settings file (i.e. "C:/projects/settings/data.xml"). */
	void							appendSettings(const std::string& name, const std::string& filename);

	/** Convenience to load a text cfg file into a collection of cfg objects.
		It will be loaded from all appropriate locations.
		\param filename is the leaf path of the settings file (i.e. "text.xml"). */	
	void							loadText(const std::string& filename, ds::Engine& engine);

private:
	EngineCfg(const EngineCfg&);
	// Make it easy for clients to access the engine settings.
	ds::cfg::Settings&				mEngineSettings;
	std::map<std::string, ds::cfg::Settings>
									mSettings;
	std::unordered_map<std::string, ds::cfg::Text>
									mTextCfg;

	// Empty settings for when some are missing. Here because we're getting
	// a shutdown crash with this as statics.
	ds::cfg::Settings				mEmptySettings;
	ds::cfg::Settings				mEditEmptySettings;
	ds::cfg::Text					mEmptyTextCfg;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINECFG_H_
