#pragma once

#include <map>
#include <unordered_map>
#include "ds/cfg/settings.h"
#include "ds/ui/sprite/text_defs.h"

namespace ds {
class Engine;

/**
 * \class EngineCfg
 * \brief Store all the engine configuration info.
 */
class EngineCfg {
public:
	EngineCfg(ds::cfg::Settings& engine_settings);

	/// Answer the requested settings file. In debug mode, throw
	/// if it doesn't exist. In release mode, just answer an empty one.
	ds::cfg::Settings&				getSettings(const std::string& name);

	/// Get the settings in the list AFTER the one specified by the name
	ds::cfg::Settings&				getNextSettings(const std::string& name);

	/// Answer the requested text cfg. 
	/// if it doesn't exist. In release mode, just answer an empty one.
	bool							hasTextStyle(const std::string& name) const;
	const ds::ui::TextStyle&		getTextStyle(const std::string& name) const;
	const ds::ui::TextStyle&		getDefaultTextStyle() const;
	void							setTextStyle(const std::string& name, const ds::ui::TextStyle&);

	/// DEPRECATED: use the setting in styles.xml
	/// This is a relative operation, so each time you run it it will multiply the new scale
	/// Recommend reloading the text using loadText() before setting this a second time
	///void							applyTextScale(const float theScale);

	/// Answers true if settings with given key is already loaded
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
		\param filename		the leaf path of the settings file (i.e. "text.xml").
		\param engine		Reference to engine */	
	void							loadText(const std::string& filename, ds::Engine& engine);

private:
	EngineCfg(const EngineCfg&);
	/// Make it easy for clients to access the engine settings.
	ds::cfg::Settings&				mEngineSettings;
	std::map<std::string, ds::cfg::Settings>
									mSettings;
	std::unordered_map<std::string, ds::ui::TextStyle>
									mTextStyles;
	ds::ui::TextStyle				mDefaultTextStyle;

	/// Empty settings for when some are missing. Here because we're getting
	/// a shutdown crash with this as statics.
	ds::cfg::Settings				mEmptySettings;
	ds::cfg::Settings				mEditEmptySettings;
};

} // namespace ds

