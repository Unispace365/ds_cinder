#include "private/gst_video_service.h"

#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/gst_video.h>
#include <ds/debug/logger.h>

#include <gst/gst.h>
#include "gst/gstplugin.h"
#include "gstreamer/gstreamer_env_check.h"

namespace ds {
namespace gstreamer {

static ds::gstreamer::EnvCheck  ENV_CHECK;

GstVideoService::GstVideoService(ds::Engine& e) 
	: mEngine(e)
	, mValidInstall(false)
{
	std::cout << "gst video service constructor " << std::endl;
	mEngine.registerSpriteImporter("video", [this](const std::string& typeName, ci::XmlTree& tree)->ds::ui::Sprite*{
		// just to verify
		if(typeName == "video"){
			ds::ui::GstVideo* video = new ds::ui::GstVideo(mEngine);

			for(auto it : tree.getAttributes()){
				std::string namey = it.getName();
				std::string valuey = it.getValue();

				if(namey == "src"){
					if(!valuey.empty()){
						video->loadVideo(valuey);
					}
				}
				//todo: add more stuff, yah?
			}

			return video;
		}

		return nullptr;
	});
}

GstVideoService::~GstVideoService() {
}

bool GstVideoService::getValidInstall(){
	return mValidInstall;
}

const std::string& GstVideoService::getErrorMessage(){
	return mErrorMessage;
}

void GstVideoService::start() {
	std::cout << "gst video service start " << std::endl;

	std::stringstream ss;
	ss << GST_VERSION_MAJOR << "." << GST_VERSION_MINOR << "." << GST_VERSION_MICRO;
	std::string gstVersion = ss.str();

	// Add the primary dll's to the PATH variable
	if(!ENV_CHECK.addGStreamerBinPath()){
		DS_LOG_WARNING("Couldn't find a binary directory for GStreamer! Install GStreamer version " << gstVersion);
		mValidInstall = false;
		return;
	}

	// Initialization must happen after we know about the bin path, or we'll get endless warnings about missing dll's
	GError* pError;
	int success = gst_init_check(NULL, NULL, &pError);
	if(success == FALSE){
		std::stringstream errorStream;
		errorStream << "GStreamerWrapper: failed to initialize GStreamer: " << pError->message;
		std::string errorStr = errorStream.str();
		DS_LOG_ERROR(errorStr);
		mErrorMessage = errorStr;
		mValidInstall = false;
	}

	// Set the plugin path using GStreamer's registry
	DS_LOG_INFO("Initialized GStreamer version " << gstVersion);

	GstRegistry* registery;
	registery = gst_registry_get();

	/* This is another method of setting the plugin path (instead of specifying GST_PLUGIN_PATH), but is a bit slower.
	auto rootPathy = std::getenv("GSTREAMER_1_0_ROOT_X86");
	if(rootPathy){
	std::string fullPathy = std::string(rootPathy) + "\\lib\\gstreamer-1.0";
	gboolean changed = gst_registry_scan_path(registery, fullPathy.c_str());
	if(changed){
	//std::cout << "found some stuff in the gstreamer root plugin path" << std::endl;
	} else {
	DS_LOG_WARNING("No plugins added from the default directory, check your gstreamer install for version " << gstVersion);
	}
	}
	*/

	/*
	// This is sort of like running gst-inspect-1.0.exe
	registery = gst_registry_get();
	GList *list, *g;
	list = gst_registry_get_plugin_list(registery);
	for(g = list; g; g = g->next) {
	GstPlugin *plugin = GST_PLUGIN(g->data);
	std::cout << "Plugin name: " << gst_plugin_get_name(plugin) << " version: " << gst_plugin_get_version(plugin) << std::endl;
	gst_object_unref(plugin);
	}
	g_list_free(list);
	*/

	//gst_registry_load
	auto playbinPlugin = gst_registry_find_plugin(registery, "videoconvert");
	if(playbinPlugin){
		std::string pluginVersion = gst_plugin_get_version(playbinPlugin);
		if(pluginVersion != gstVersion){
			DS_LOG_WARNING("Plugin version and compiled GStreamer version don't match. You have version " << pluginVersion << ". If you experience problems, install gstreamer " << gstVersion);
		}
		gst_object_unref(playbinPlugin);
	}

	

	mValidInstall = true;
}


} // namespace web
} // namespace ds