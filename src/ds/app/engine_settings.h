#pragma once
#ifndef DS_APP_ENGINESETTINGS_H_
#define DS_APP_ENGINESETTINGS_H_

#include "ds/config/settings.h"

namespace ds {

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
 * By default, there is a single settings file named engine_settings.xml
 * located in the settings/ directory below the application folder.
 *
 * The command line can be used to provide the location of one or both of
 * these files:
 *   APP_SETTINGS="filename.xml" to provide the filename for the app settings.
 *   LOCAL_SETTINGS="filename.xml" to provide the filename for the local
 * settings.  The app settings will need a project_path setting to provide
 * the path to locate this file.  The root is always Documents\downstream\settings,
 * then the project_path, then the filename.
 *   LOCAL_PATH="project_path\filename.xml" Using this variant, you automatically
 * assign a project path to the system that will overwrite any project_path
 * specified in the app settings.
 */
class EngineSettings : public ds::cfg::Settings {
  public:
    EngineSettings(const std::string& applicationPath);
};

} // namespace ds

#endif // DS_APP_ENGINESETTINGS_H_