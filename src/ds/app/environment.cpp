#include "ds/app/environment.h"

#include <Poco/File.h>
#include <Poco/Path.h>

static std::string    folder_from(const Poco::Path&, const std::string& folder);

namespace ds {

/**
 * \class ds::Environment
 */
Environment::Environment()
{
}

std::string Environment::getAppFolder(const std::string& appPath, const std::string& folderName)
{
  Poco::Path      p(appPath);
  std::string     ans;
  // A couple things limit the search -- the directory can't get
  // too short, and right now nothing is more then 3 steps from the appPath.
  int             count = 0;
  while ((ans=folder_from(p, folderName)).empty()) {
    p.popDirectory();
    if (count++ >= 3 || p.depth() < 2) return "";
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

std::string Environment::getProjectSettingsFolder(const std::string& projectPath)
{
	Poco::Path			p(getDownstreamDocumentsFolder());
	p.append("settings");
	if (!projectPath.empty()) p.append(projectPath);
	return p.toString();
}

} // namespace ds

static std::string    folder_from(const Poco::Path& parentP, const std::string& folder)
{
  Poco::Path          p(parentP);
  p.append(folder);

  const Poco::File    f(p);
  if (f.exists() && f.isDirectory()) return f.path();
  return "";
}
