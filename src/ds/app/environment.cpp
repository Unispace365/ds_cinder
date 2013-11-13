#include "ds/app/environment.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include "ds/app/app.h"
#include "ds/app/engine/engine_settings.h"

static std::string    folder_from(const Poco::Path&, const std::string& folder, const std::string& fileName);

namespace ds {

/**
 * \class ds::Environment
 */
Environment::Environment()
{
}

const std::string& Environment::SETTINGS()
{
  static const std::string    SZ("settings");
  return SZ;
}

const std::string& Environment::RESOURCES()
{
  static const std::string    SZ("resources");
  return SZ;
}

std::string Environment::getAppFolder(const std::string& folderName, const std::string& fileName, const bool verify)
{
  Poco::Path      p(ds::App::envAppPath());
  std::string     ans;
  // A couple things limit the search -- the directory can't get
  // too short, and right now nothing is more then 3 steps from the appPath.
  int             count = 0;
  while ((ans=folder_from(p, folderName, fileName)).empty()) {
    p.popDirectory();
    if (count++ >= 3 || p.depth() < 2) return "";
  }
	if (verify) {
		try {
			Poco::File		f(ans);
			if (!f.exists()) return "";
		} catch (std::exception const&) {
		}
	}
  return ans;
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

#if 0
// Answer the settings folder for this project.
//    static std::string		      getProjectSettingsFolder(const std::string& projectPath);
std::string Environment::getProjectSettingsFolder(const std::string& projectPath)
{
	Poco::Path			p(getDownstreamDocumentsFolder());
	p.append(SETTINGS());
	if (!projectPath.empty()) p.append(projectPath);
	return p.toString();
}
#endif

std::string Environment::getLocalSettingsPath(const std::string& fileName)
{
  if (EngineSettings::envProjectPath().empty()) return "";
  Poco::Path			p(getDownstreamDocumentsFolder());
  p.append(SETTINGS()).append(EngineSettings::envProjectPath()).append(fileName);
  return p.toString();
}

void Environment::loadSettings(const std::string& filename, ds::cfg::Settings& settings)
{
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

} // namespace ds

static std::string    folder_from(const Poco::Path& parentP, const std::string& folder, const std::string& fileName)
{
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
