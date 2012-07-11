#pragma once
#ifndef DS_APP_ENVIRONMENT_H_
#define DS_APP_ENVIRONMENT_H_

#include <string>

namespace ds {

/**
 * \class ds::Environment
 * Access to the environment data, i.e. file paths etc.
 * Ideally I'd use boost fs::path but I'm keeping it simple as I port.
 */
class Environment {
  public:
    // Answer an app folder -- currently only "settings" is valid for arg 2.
    // This function assumes that I don't actually know the location of the folderName
    // relative to my appPath, and searches up the appPath looking for it.  This makes
    // it so no configuration is needed between dev and production environments.
    static std::string    getAppFolder(const std::string& appPath, const std::string& folderName);
    // Answer the Downstream documents folder.
    static std::string		getDownstreamDocumentsFolder();
    // Answer the settings folder for this project.
    static std::string		getProjectSettingsFolder(const std::string& projectPath);

    Environment();
};

} // namespace ds

#endif // DS_APP_ENVIRONMENT_H_