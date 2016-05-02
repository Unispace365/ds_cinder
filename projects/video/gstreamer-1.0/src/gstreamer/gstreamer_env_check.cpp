#include "gstreamer_env_check.h"

#include <Poco/Path.h>

#include <regex>

#include <cinder/Filesystem.h>
#include <ds/util/file_meta_data.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

namespace {
// Replaces forward slash with back slash and replaces all multiple repeats of back slashes with a single back slash
void normalizePath(std::string& path) {
	try {
		static std::regex path_anomalies("\\\\+|/");
		path = std::regex_replace(path, path_anomalies, "/");
		path = Poco::Path::expand(path);
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

bool EnvCheck::addGStreamerBinPath(){
	std::string gstreamer_path = getEnv("GSTREAMER_1_0_ROOT_X86");
	std::string gstreamer_bin_path = gstreamer_path + "\\bin";
	normalizePath(gstreamer_bin_path);
	std::string path_variable{ getEnv("PATH") };


	if(path_variable.find(gstreamer_bin_path) == std::string::npos) {
		ds::Environment::addToFrontEnvironmentVariable("PATH", gstreamer_bin_path);
	}
		
	if(getEnv("GST_PLUGIN_PATH").empty()){
		std::string gstreamer_plugin_path = gstreamer_path + "\\lib\\gstreamer-1.0";
		normalizePath(gstreamer_plugin_path);

		ds::Environment::addToEnvironmentVariable("GST_PLUGIN_PATH", gstreamer_plugin_path);
	}

	if(ds::FileMetaData::safeFileExistsCheck(gstreamer_path, true)){
		return true;
	} 
	
	return false;	

}

}} //!ds::gstreamer
