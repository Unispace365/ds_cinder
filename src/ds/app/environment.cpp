#include "ds/app/environment.h"

#include <boost/algorithm/string.hpp>
#include <Poco/File.h>
#include <Poco/Path.h>
#include "ds/app/app.h"
#include "ds/app/engine/engine_settings.h"

static std::string    folder_from(const Poco::Path&, const std::string& folder, const std::string& fileName);

namespace ds {

/**
 * \class ds::Environment
 */
Environment::Environment() {
}

const std::string& Environment::SETTINGS() {
	static const std::string    SZ("settings");
	return SZ;
}

const std::string& Environment::RESOURCES() {
	static const std::string    SZ("resources");
	return SZ;
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

std::string Environment::getLocalResourcesFolder( const std::string& folderName, const std::string& fileName /*= ""*/ )
{
  if (EngineSettings::envProjectPath().empty())
    return "";
  Poco::Path p(getDownstreamDocumentsFolder());
  p.append(RESOURCES()).append(EngineSettings::envProjectPath());
  std::string     ans;
  // A couple things limit the search -- the directory can't get
  // too short, and right now nothing is more then 3 steps from the appPath.
  int             count = 0;
  while ((ans=folder_from(p, folderName, fileName)).empty()) {
    p.popDirectory();
    if (count++ >= 3 || p.depth() < 2)
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
  if (EngineSettings::envProjectPath().empty()) return "";
  Poco::Path			p(getDownstreamDocumentsFolder());
  p.append(SETTINGS()).append(EngineSettings::envProjectPath()).append(fileName);
  return p.toString();
}

void Environment::loadSettings(const std::string& filename, ds::cfg::Settings& settings) {
	settings.readFrom(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), filename), false);
	settings.readFrom(ds::Environment::getLocalSettingsPath(filename), true);
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

std::string Environment::expand(const std::string& _path) {
	std::string		p(_path);
	boost::replace_all(p, "%APP%", ds::App::envAppDataPath());
	return p;
}

void Environment::addToEnvironmentVariable(const std::string& variable, const std::string& value) {
	std::string		new_path(variable + "=");
	const char*		path_env = getenv(variable.c_str());
	if (path_env) {
		new_path += path_env;
	}
	if (new_path.length() > 0) {
		if (!(new_path.back() == '=' || new_path.back() == ';')) {
			new_path += ";";
		}
	}
	new_path += value;
	_putenv(new_path.c_str());
}

} // namespace ds

static std::string    folder_from(const Poco::Path& parentP, const std::string& folder, const std::string& fileName) {
	Poco::Path          p(parentP);
	p.append(folder);

	const Poco::File    f(p);
	if (f.exists() && f.isDirectory()) {
		if (fileName.empty()) return f.path();
		p.append(fileName);
		return p.toString();
	}
	return "";
}
