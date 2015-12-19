#pragma once
#ifndef DS_APP_ENGINE_ENGINESETTINGS_H_
#define DS_APP_ENGINE_ENGINESETTINGS_H_

#include "ds/cfg/settings.h"

namespace ds {
class Environment;

/**
 * \class ds::EngineSettings
 * Container for the engine settings.  This class only adds initialization
 * behavior, and codifies the rules around the engine settings.
 *
 * RULES:
 * Engine settings are loaded from two files, an app file and a local
 * file.  The local file is optional, and if it exists, is layered on top
 * of the app, replacing existing items and adding any new ones.
 *
 * By default, there is a single settings file named engine.xml
 * located in the settings/ directory below the application folder.
 *
 * The command line can be used to provide the location of one or both of
 * these files:
 * \code
 *   APP_SETTINGS="filename.xml" //to provide the filename for the app settings.
 *   LOCAL_SETTINGS="filename.xml" //to provide the filename for the local
 * \endcode
 * settings.  The app settings will need a project_path setting to provide
 * the path to locate this file.  The root is always \code Documents\downstream\settings \endcode
 * then the project_path, then the filename.
 *   \code LOCAL_PATH="project_path\filename.xml" \endcode Using this variant, you automatically
 * assign a project path to the system that will overwrite any project_path
 * specified in the app settings.
 */
class EngineSettings : public ds::cfg::Settings {
public:
	EngineSettings();

private:
	friend class Environment;
	friend class EngineCfg;
	static const std::string&		envProjectPath();
	// Answer the configuration.xml if it exists.
	static const ds::cfg::Settings&	getConfiguration();
	static const std::string&		getConfigurationFolder();
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINESETTINGS_H_