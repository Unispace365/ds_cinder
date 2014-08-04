#pragma once
#ifndef DS_APP_ENVIRONMENT_H_
#define DS_APP_ENVIRONMENT_H_

#include <string>

namespace ds {

namespace cfg {
class Settings;
}

/**
 * \class ds::Environment
 * Access to the environment data, i.e. file paths etc.
 * Ideally I'd use boost fs::path but I'm keeping it simple as I port.
 */
class Environment {
public:
	// Return the same path but with any environment variables expanded. Current variables:
	//	%APP% -- expanded to app folder
	//	%LOCAL% -- expanded to downstream documents folder
	//  %PP% -- expand the project path, i.e. "%LOCAL%/cache/%PP%/images/"
	static std::string			expand(const std::string& path);

	// Answer an app folder -- currently only SETTINGS() is valid for arg 1.
	// If fileName is valid, then it will be appended to the found app folder, if it exists.
	// This function assumes that I don't actually know the location of the folderName
	// relative to my appPath, and searches up the appPath looking for it.  This makes
	// it so no configuration is needed between dev and production environments.
	// If verify is true, then verify that the folder or file exists, otherwise answer a blank string.
	static std::string			getAppFolder(	const std::string& folderName, const std::string& fileName = "",
												const bool verify = false);

	// Answer a complete path to a local settings file.  Supply an empty file name
	// to just get the local settings folder.
	static std::string			getLocalSettingsPath(const std::string& fileName);
	// Convenience to load in a settings file, first from the app path, then the local path
	static void					loadSettings(const std::string& filename, ds::cfg::Settings&);
		
	// Utility to add a value to an environment variable. Probably this is a bad place
	// for this, and it should be hidden in the app.
	static void					addToEnvironmentVariable(const std::string& variable, const std::string& value);


	// DEPRECATED
	static const std::string&	SETTINGS();
	static const std::string&	RESOURCES();
	// Answer the Downstream documents folder.
	// Obsolete -- use ds::Environment::expand("%LOCAL%");
	static std::string			getDownstreamDocumentsFolder();
	// Answer the current project path
	// Obsolete -- use ds::Environment::expand("%PP%");
	static std::string			getProjectPath();
	// Path is relative to the app folder. It can end in a folder or file leaf.
	// Obsolete -- use ds::Environment::expand("%APP%/path");
	static std::string			getAppFile(const std::string& path);
	// Obsolete -- use ds::Environment::expand("%LOCAL%/resources/folderName/fileName");
	static std::string			getLocalResourcesFolder(const std::string& folderName, const std::string& fileName = "");
	// OK, this API has become pretty messy. This gets to the local folder, which can have
	// optional category (SETTINGS() etc.), project_path, and file name
	// Obsolete -- use ds::Environment::expand("%LOCAL%/settings/%PP%/filename");
	static std::string			getLocalFile(const std::string& category, const bool includeProjectPath, const std::string& filename);

private:
	Environment();
};

} // namespace ds

#endif // DS_APP_ENVIRONMENT_H_