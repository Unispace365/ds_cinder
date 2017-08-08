#include "stdafx.h"

#include "ds/app/engine/engine_settings.h"

#include <Poco/Path.h>
#include <Poco/String.h>
#include "ds/app/environment.h"
#include "ds/util/string_util.h"
#include "ds/debug/logger.h"
#include "ds/util/file_meta_data.h"

namespace {
std::string			PROJECT_PATH;
// The configuration settings
std::string			CONFIGURATION_FOLDER;

static bool			get_key_value(const std::string& arg, std::string& key, std::string& value) {
	const std::string	SEP_SZ("=");
	const std::string	SEP_SZ_ALT(":");
	size_t				sep = arg.find(SEP_SZ);
	if(sep == arg.npos) {
		sep = arg.find(SEP_SZ_ALT);
		if(sep == arg.npos)
			return false;
	}

	key = arg.substr(0, sep);
	Poco::toLowerInPlace(key);

	value = arg.substr(sep + 1, arg.size() - (sep + 1));

	return true;
}

} // anonymous namespace

namespace ds {

/**
 * \class ds::EngineSettings
 */
EngineSettings::EngineSettings() 
	: mLoadedAnySettings(false)
{
	setName("engine");

	mLoadedAnySettings = false;
	mStartupInfo.str("");

	// Default file names.
	const std::string			DEFAULT_FILENAME("engine.xml");
	std::string					appFilename = DEFAULT_FILENAME,
		localFilename,
		commandLineAppConfig,
		projectPath;

	std::vector<std::string>	args = ds::Environment::getCommandLineParams();
	for(const auto& arg : args) {
		std::string				key, value;
		if(!get_key_value(arg, key, value)) continue;

		if(key == "app_settings") {
			appFilename = value;
			if(localFilename.empty()) localFilename = value;
		} else if(key == "local_settings") {
			localFilename = value;
		} else if(key == "local_path") {
			// The local path and filename need to be parsed here.
			Poco::Path				p(value);
			const std::string&		fn(p.getFileName());
			const std::string		full(p.toString());
			projectPath = full.substr(0, full.length() - (fn.length() + 1));
			localFilename = fn;
		}
		else if (key == "configuration" || key == "config") {
			commandLineAppConfig = value;
		}

	}
	if(localFilename.empty()) localFilename = DEFAULT_FILENAME;

	// I have all the argument-supplied paths and filenames.  Now I can
	// start reading my settings files.

	// APP SETTINGS
	// Find my app settings/ directory.  This will vary based on whether I'm in a dev environment or
	// in a production, but I will have a settings/ folder either at or above me.
	const std::string         appSettingsPath = ds::Environment::getAppFolder(ds::Environment::SETTINGS());
	if(appSettingsPath.empty()){
		//throw std::runtime_error("Missing application settings folder");
		std::cout << "Couldn't find the application settings folder, that could be a problem." << std::endl;
	}
	Poco::Path                appP(appSettingsPath);
	appP.append(appFilename);

	std::string appFullPath = appP.toString();
	if(safeFileExistsCheck(appFullPath)){
		mLoadedAnySettings = true;
		mStartupInfo << "EngineSettings: Reading app settings from " << appFullPath << std::endl;
		readFrom(appFullPath, false);
	}

	// LOCAL SETTINGS
	// The project path is taken from the supplied arguments, if it existed, or else it's
	// pulled from the settings I just loaded.  If neither has it, then I guess no local settings.
	if(projectPath.empty()) {
		projectPath = getString("project_path", 0, projectPath);
	} else {
		// If it exists, then make sure then any project_path in the settings is the same.  No one
		// should ever use that, but let's be safe.
		
		auto& theSetting = getSetting("project_path", 0);
		theSetting.mRawValue = projectPath;

	}

	if(!projectPath.empty()) {
		// Set the global project path
		PROJECT_PATH = projectPath;

		std::string localSettingsPath = ds::Environment::getLocalSettingsPath(localFilename);
		if(safeFileExistsCheck(localSettingsPath)){
			mLoadedAnySettings = true;
			mStartupInfo << "EngineSettings: Reading app settings from " << localSettingsPath << std::endl;
			readFrom(localSettingsPath, true);
		}

		// Load the configuration settings, which can be used to modify settings even more.
		// Currently used to provide alternate layout sizes.
		ds::Environment::loadSettings("configuration", "configuration.xml", mConfiguration);
		CONFIGURATION_FOLDER = commandLineAppConfig.empty()
			? mConfiguration.getString("folder", 0, "")
			: commandLineAppConfig;

		// If the folder exists, then apply any changes to the engine file
		if(!CONFIGURATION_FOLDER.empty()) {
			const std::string		app = ds::Environment::expand("%APP%/settings/%CFG_FOLDER%/" + appFilename);
			const std::string		local = ds::Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%/" + appFilename);

			if(safeFileExistsCheck(app)){
				mLoadedAnySettings = true;
				mStartupInfo << "EngineSettings: Reading app settings from " << app << std::endl;
				readFrom(app, true);
			}

			if(safeFileExistsCheck(local)){
				mLoadedAnySettings = true;
				mStartupInfo << "EngineSettings: Reading app settings from " << local << std::endl;
				readFrom(local, true);
			}
		}
	}

}

const std::string& EngineSettings::envProjectPath() {
	return PROJECT_PATH;
}

const ds::cfg::Settings& EngineSettings::getConfiguration() {
	return mConfiguration;
}

const std::string& EngineSettings::getConfigurationFolder() {
	return CONFIGURATION_FOLDER;
}

void EngineSettings::printStartupInfo(){
	if(mLoadedAnySettings){
		DS_LOG_INFO(std::endl << mStartupInfo.str());
	} else {
		DS_LOG_WARNING("No settings files loaded. Using default values.");
	}
}

} // namespace ds

