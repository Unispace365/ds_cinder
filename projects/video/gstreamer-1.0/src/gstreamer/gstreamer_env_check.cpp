#include "gstreamer_env_check.h"

#include <Poco/Path.h>

#include <regex>

#include <cinder/Filesystem.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>

namespace {
// Replaces forward slash with back slash and replaces all multiple repeats of back slashes with a single back slash
void normalizePath(std::string& path) {
    try {
        static std::regex path_anomalies("\\\\+|/");
        path = std::regex_replace(path, path_anomalies, "/");
    }
    catch (...) {}
}
// Long story short, this is here to prevent a Release mode exception in VS2013
std::string getEnv(const std::string& name) {
    auto _env_var_ptr = std::getenv(name.c_str());
    return std::string{ _env_var_ptr == nullptr ? "" : _env_var_ptr };
}}

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
    std::string _gstreamer_path{ getEnv(mGstreamerEnvVarName) };

    // If GStreamer is not set...
    if (_gstreamer_path.empty() && !ci::fs::exists(mGstreamerDefaultLoc))
    {
        DS_LOG_FATAL("GStreamer could not be found. "
            << "Please install GStreamer and set "
            << "DS_CINDER_GSTREAMER_1-0 env var.");
    }
    else
    {
        std::string _path_variable{ getEnv("PATH") };
        
        if (_gstreamer_path.empty())
        {
            DS_LOG_WARNING("GStreamer path not found. setting to default location.");
            _gstreamer_path = mGstreamerDefaultLoc;
        }

        auto _gstreamer_binary_path = (_gstreamer_path + "\\bin");
        normalizePath(_gstreamer_binary_path);

        // Add binary dir to path
        if (_path_variable.find(_gstreamer_binary_path) == std::string::npos)
        {
            ds::Environment::addToFrontEnvironmentVariable("PATH", Poco::Path::expand(_gstreamer_binary_path));
        }

        if (getEnv(mGstreamerPluginPath).empty())
        {
            auto _gstreamer_plugin_path = (_gstreamer_path + "\\lib\\gstreamer-1.0");
            normalizePath(_gstreamer_plugin_path);

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

}} //!ds::gstreamer
