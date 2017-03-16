#include "stdafx.h"

#include "ds/app/environment.h"

#include <boost/algorithm/string.hpp>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Environment.h>

#include "ds/app/app.h"
#include "ds/app/engine/engine_settings.h"

#ifdef CINDER_MSW
#include "cinder/Clipboard.h"
#else
#include "glfw/glfw3.h"
#include "glfw/glfw3native.h"
#endif

static std::string    folder_from(const Poco::Path&, const std::string& folder, const std::string& fileName);

namespace ds {

namespace {
std::string				DOCUMENTS("%DOCUMENTS%");
#ifdef WIN32
const std::string		ENV_PATH_SEPARATOR = ";";
#else
const std::string		ENV_PATH_SEPARATOR = ":";
#endif
}

bool Environment::initialize() {
	// We will need to do something different for linux, no doubt
	Poco::Path			p(Poco::Path::expand("%USERPROFILE%"));
	p.append("Documents");
	DOCUMENTS = p.toString();
	return true;
}

const std::string& Environment::SETTINGS() {
	static const std::string    SZ("settings");
	return SZ;
}

const std::string& Environment::RESOURCES() {
	static const std::string    SZ("resources");
	return SZ;
}

std::string Environment::expand(const std::string& _path) {
	std::string		p(_path);
	boost::replace_all(p, "%APP%", ds::App::envAppDataPath());
	boost::replace_all(p, "%PP%", EngineSettings::envProjectPath());
	boost::replace_all(p, "%LOCAL%", getDownstreamDocumentsFolder());
	boost::replace_all(p, "%CFG_FOLDER%", EngineSettings::getConfigurationFolder());
	boost::replace_all(p, "%DOCUMENTS%", DOCUMENTS);
	// This can result in double path separators, so flatten
	return Poco::Path(p).toString();
}


std::string Environment::contract(const std::string& fullPath){
	std::string		p(fullPath);
	boost::replace_all(p, ds::App::envAppDataPath(), "%APP%");
	boost::replace_all(p, EngineSettings::envProjectPath(), "%PP%");
	boost::replace_all(p, getDownstreamDocumentsFolder(), "%LOCAL%");
	boost::replace_all(p, EngineSettings::getConfigurationFolder(), "%CFG_FOLDER%");
	boost::replace_all(p, DOCUMENTS, "%DOCUMENTS%");
	// This can result in double path separators, so flatten
	return Poco::Path(p).toString();
}

std::string Environment::getAppFolder(const std::string& folderName, const std::string& fileName, const bool verify) {
	Poco::Path				p(ds::App::envAppDataPath());
	if (!folderName.empty()) p.append(folderName);
	if (!fileName.empty()) p.append(fileName);
	const std::string		ans(p.toString());
	if (verify) {
		try {
			Poco::File		f(ans);
			if (!f.exists()) return "";
		} catch (std::exception const&) {
		}
	}
	return ans;
}

std::string Environment::getAppFile(const std::string& path) {
	Poco::Path      p(ds::App::envAppDataPath());
	p.append(path);
	return p.toString();
}

std::string Environment::getLocalResourcesFolder(const std::string& folderName, const std::string& fileName /*= ""*/)
{
	if(EngineSettings::envProjectPath().empty()){
		return "";
	}
	Poco::Path p(getDownstreamDocumentsFolder());
	p.append(RESOURCES()).append(EngineSettings::envProjectPath());
	std::string     ans;
	// A couple things limit the search -- the directory can't get
	// too short, and right now nothing is more then 3 steps from the appPath.
	int             count = 0;
	while((ans = folder_from(p, folderName, fileName)).empty()) {
		p.popDirectory();
		if(count++ >= 3 || p.depth() < 2)
			return "";
	}
	return ans;
}

std::string Environment::getDownstreamDocumentsFolder()
{
	// We will need to do something different for linux, no doubt
	Poco::Path			p(Poco::Path::expand("%USERPROFILE%"));
	p.append("Documents");
	p.append("downstream");
	return p.toString();
}

std::string Environment::getLocalSettingsPath(const std::string& fileName)
{
	if(EngineSettings::envProjectPath().empty()) return "";
	Poco::Path			p(getDownstreamDocumentsFolder());
	p.append(SETTINGS()).append(EngineSettings::envProjectPath()).append(fileName);
	return p.toString();
}

void Environment::loadSettings(const std::string& filename, ds::cfg::Settings& settings) {
	settings.readFrom(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), filename), false);
	settings.readFrom(ds::Environment::getLocalSettingsPath(filename), true);
	if (!ds::EngineSettings::getConfigurationFolder().empty()) {
		const std::string		app = ds::Environment::expand("%APP%/settings/%CFG_FOLDER%/" + filename);
		const std::string		local = ds::Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%/" + filename);
		settings.readFrom(app, true);
		settings.readFrom(local, true);
	}
}

void Environment::saveSettings(const std::string& filename, ds::cfg::Settings& settings) {
	if(!ds::EngineSettings::getConfigurationFolder().empty()) {
		const std::string		local = ds::Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%/" + filename);
		settings.writeTo(local);
	}
}


std::string Environment::getLocalFile(	const std::string& category,
										const bool includeProjectPath,
										const std::string& filename)
{
	Poco::Path			p(getDownstreamDocumentsFolder());
	if (!category.empty()) {
		p.append(category);
		if (includeProjectPath && !EngineSettings::envProjectPath().empty()) {
			p.append(EngineSettings::envProjectPath());
			if (!filename.empty()) {
				p.append(filename);
			}
		}
	}
	return p.toString();
}

std::string Environment::getProjectPath() {
	return EngineSettings::envProjectPath();
}

void Environment::addToEnvironmentVariable(const std::string& variable, const std::string& value) {
	std::string new_path = Poco::Environment::get(variable, "");

	if (new_path.length() == 0)
		new_path += ENV_PATH_SEPARATOR;
	new_path += value;

	Poco::Environment::set(variable, new_path);
}

void Environment::addToFrontEnvironmentVariable(const std::string& variable, const std::string& value) {
	std::string old_path = Poco::Environment::get(variable, "");
	std::string new_path = value + ENV_PATH_SEPARATOR + old_path;
	Poco::Environment::set(variable, new_path);
}


std::string Environment::getClipboard() {
#ifdef CINDER_MSW
	return ci::Clipboard::getString();
#else
	auto window = (GLFWwindow*)ci::app::getWindow()->getNative();
	return std::string( glfwGetClipboardString(window) );
#endif
}


} // namespace ds

static std::string    folder_from(const Poco::Path& parentP, const std::string& folder, const std::string& fileName) {
	Poco::Path          p(parentP);
	p.append(folder);

	const Poco::File    f(p);
	if(f.exists() && f.isDirectory()) {
		if(fileName.empty()) return f.path();
		p.append(fileName);
		return p.toString();
	}
	return "";
}
