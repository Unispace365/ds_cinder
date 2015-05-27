#include "gstreamer_env_check.h"

#include <Poco/Path.h>

#include <boost/algorithm/string/replace.hpp>

#include <cinder/Filesystem.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>

namespace ds {
namespace gstreamer {

void EnvCheck::setEnvVarName(const std::string& name)
{
    mGstreamerEnvVarName = name;
}

void EnvCheck::setDefaultLoc(const std::string& loc)
{
    mGstreamerDefaultLoc = loc;
}

void EnvCheck::verifyPathVar()
{
    // Get the GST environment variable
    std::string _gstreamer_path{ std::getenv(mGstreamerEnvVarName.c_str()) };

    // If GStreamer is not set...
    if (_gstreamer_path.empty() && !ci::fs::exists(mGstreamerDefaultLoc))
    {
        DS_LOG_FATAL("GStreamer could not be found. "
            << "Please install GStreamer and set "
            << "DS_CINDER_GSTREAMER_1-0 env var.");
    }
    else
    {
        std::string _path_variable{ std::getenv("PATH") };
        
        if (_gstreamer_path.empty())
        {
            DS_LOG_WARNING("GStreamer path not found. setting to default location.");
            _gstreamer_path = mGstreamerDefaultLoc;
        }

        auto _gstreamer_binary_path = (_gstreamer_path + "\\bin");
        // this could be a regex but pfft. whatever.
        boost::algorithm::replace_all(_gstreamer_binary_path, "/", "\\");
        boost::algorithm::replace_all(_gstreamer_binary_path, "\\\\", "\\");

        // Add binary dir to path
        if (_path_variable.find(_gstreamer_binary_path) == std::string::npos)
        {
            ds::Environment::addToFrontEnvironmentVariable("PATH", Poco::Path::expand(_gstreamer_binary_path));
        }

        if (!std::getenv(mGstreamerPluginPath.c_str()))
        {
            auto _gstreamer_plugin_path = (_gstreamer_path + "\\lib\\gstreamer");
            // this could be a regex but pfft. whatever (#2).
            boost::algorithm::replace_all(_gstreamer_plugin_path, "/", "\\");
            boost::algorithm::replace_all(_gstreamer_plugin_path, "\\\\", "\\");

            ds::Environment::addToEnvironmentVariable(mGstreamerPluginPath, _gstreamer_plugin_path);
        }
    }
}

EnvCheck::EnvCheck()
    : mGstreamerEnvVarName("DS_CINDER_GSTREAMER_1-0")
    , mGstreamerDefaultLoc("C:\\gstreamer\\1.0\\x86")
    , mGstreamerPluginPath("GST_PLUGIN_PATH")
{
    verifyPathVar();
}

}
}