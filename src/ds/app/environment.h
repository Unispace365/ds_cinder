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
    static const std::string&   SETTINGS();
    static const std::string&   RESOURCES();
    // Answer an app folder -- currently only SETTINGS() is valid for arg 1.
    // If fileName is valid, then it will be appended to the found app folder, if it exists.
    // This function assumes that I don't actually know the location of the folderName
    // relative to my appPath, and searches up the appPath looking for it.  This makes
    // it so no configuration is needed between dev and production environments.
    static std::string          getAppFolder(const std::string& folderName, const std::string& fileName = "");
    // Answer the Downstream documents folder.
    static std::string		      getDownstreamDocumentsFolder();
    // Answer a complete path to a local settings file.  Supply an empty file name
    // to just get the local settings folder.
    static std::string		      getLocalSettingsPath(const std::string& fileName);

  private:
    Environment();
};

} // namespace ds

#endif // DS_APP_ENVIRONMENT_H_