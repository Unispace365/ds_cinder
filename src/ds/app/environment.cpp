#include "ds/app/environment.h"

#include <Poco/Path.h>

namespace ds {

/**
 * \class ds::Environment
 */
Environment::Environment()
{
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
