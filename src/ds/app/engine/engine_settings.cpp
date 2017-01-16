#include "stdafx.h"

#include "ds/app/engine/engine_settings.h"

#include <Poco/Path.h>
#include <Poco/String.h>
#include "ds/app/environment.h"
#include "ds/util/string_util.h"
#include "ds/debug/logger.h"
#include "ds/util/file_meta_data.h"

static bool			get_key_value(const std::wstring& arg, std::string& key, std::string& value);
static void			get_cmd_line(std::vector<std::wstring>&);

namespace {
std::string			PROJECT_PATH;
// The configuration settings
ds::cfg::Settings	CONFIGURATION_SETTINGS;
std::string			CONFIGURATION_FOLDER;
}

namespace ds {

/**
 * \class ds::EngineSettings
 */
EngineSettings::EngineSettings() 
	: mLoadedAnySettings(false)
{

	mLoadedAnySettings = false;
	mStartupInfo.str("");

	// Default file names.
	const std::string			DEFAULT_FILENAME("engine.xml");
	std::string					appFilename = DEFAULT_FILENAME,
		localFilename,
		projectPath;

	std::vector<std::wstring>	args;
	get_cmd_line(args);
	for(auto it = args.begin(), end = args.end(); it != end; ++it) {
		std::string				key, value;
		if(!get_key_value(*it, key, value)) continue;

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
	}
	if(localFilename.empty()) localFilename = DEFAULT_FILENAME;

	// I have all the argument-supplied paths and filenames.  Now I can
	// start reading my settings files.

	// APP SETTINGS
	// Find my app settings/ directory.  This will vary based on whether I'm in a dev environment or
	// in a production, but I will have a settings/ folder either at or above me.
	const std::string         appSettingsPath = ds::Environment::getAppFolder(ds::Environment::SETTINGS());
	if(appSettingsPath.empty()) throw std::runtime_error("Missing application settings folder");
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
		projectPath = getText("project_path", 0, projectPath);
	} else {
		// If it exists, then make sure then any project_path in the settings is the same.  No one
		// should ever use that, but let's be safe.
		ds::cfg::Settings::Editor   ed(*this, Editor::SET_MODE);
		ed.setText("project_path", projectPath);
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
		ds::Environment::loadSettings("configuration.xml", CONFIGURATION_SETTINGS);
		CONFIGURATION_FOLDER = CONFIGURATION_SETTINGS.getText("folder", 0, "");

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
	return CONFIGURATION_SETTINGS;
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


static bool       get_key_value(const std::wstring& arg, std::string& key, std::string& value)
{
	const std::wstring    SEP_SZ(L"=");
	const std::wstring    SEP_SZ_ALT(L":");
	size_t                sep = arg.find(SEP_SZ);
	if(sep == arg.npos) {
		sep = arg.find(SEP_SZ_ALT);
		if(sep == arg.npos)
			return false;
	}

	key = ds::utf8_from_wstr(arg.substr(0, sep));
	Poco::toLowerInPlace(key);

	value = ds::utf8_from_wstr(arg.substr(sep + 1, arg.size() - (sep + 1)));

	return true;
}

// Platform dependent code for getting the command line args

#if defined( CINDER_MSW )
#include <winsock2.h> // need to include winsock2 before windows
#include <windows.h>
#include <shellapi.h>

static void       get_cmd_line(std::vector<std::wstring>& out)
{
	int nArgs;
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if(szArglist != NULL) {
		for(int i = 0; i < nArgs; ++i) out.push_back(szArglist[i]);
	}
	LocalFree(szArglist);
}
#else
static void get_cmd_line(std::vector<std::wstring>& out)
{
  std::cout << "ds::EngineSettings get_cmd_line() unimplemented on this platform" << std::endl;
}
#endif
