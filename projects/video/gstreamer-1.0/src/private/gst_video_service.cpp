#include "stdafx.h"

#include "private/gst_video_service.h"

#include <ds/app/engine/engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/gst_video.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/string_util.h>

#include <gst/gst.h>
#include "gst/gstplugin.h"
#include "gstreamer/gstreamer_env_check.h"

namespace ds {
namespace gstreamer {

static ds::gstreamer::EnvCheck ENV_CHECK;

GstVideoService::GstVideoService(ds::Engine& e)
  : mEngine(e)
  , mValidInstall(false) {
	mEngine.registerSpriteImporter(
		"video", [this](ds::ui::SpriteEngine& engine) -> ds::ui::Sprite* { return new ds::ui::GstVideo(mEngine); });

	mEngine.registerSpritePropertySetter(
		"video_src", [this](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer) {
			std::string		  absPath  = ds::filePathRelativeTo(fileReferrer, theValue);
			ds::ui::GstVideo* gstVideo = dynamic_cast<ds::ui::GstVideo*>(&theSprite);
			if (!gstVideo) {
				DS_LOG_WARNING("Tried to set the property video_src on a non-GstVideo sprite");
				return;
			}

			gstVideo->loadVideo(absPath);
		});

	mEngine.registerSpritePropertySetter(
		"stream_src", [this](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer) {
			auto			  streamTokens = ds::split(theValue, "; ", true);
			ds::ui::GstVideo* gstVideo	 = dynamic_cast<ds::ui::GstVideo*>(&theSprite);
			if (!gstVideo) {
				DS_LOG_WARNING("Tried to set the property stream_src on a non-GstVideo sprite");
				return;
			}
			if (streamTokens.size() < 3) {
				DS_LOG_WARNING("Not enough parameters to load a video stream from xml parameters.");
			} else {
				std::string pipeline = streamTokens[0];
				const float widdy	= ds::string_to_float(streamTokens[1]);
				const float hiddy	= ds::string_to_float(streamTokens[2]);
				if (pipeline.empty() || widdy < 1 || hiddy < 1) {
					DS_LOG_WARNING("Incorrect parameters for xml-loaded video stream. pipeline=" << pipeline << ", w=" << widdy
																								 << ", h=" << hiddy);
				} else {
					gstVideo->startStream(pipeline, widdy, hiddy);
				}
			}
	});


	mEngine.registerSpritePropertySetter<ds::ui::GstVideo>(
		"video_gl_mode",
		[](ds::ui::GstVideo& video, const std::string& theValue, const std::string& fileReferrer) {
		if(ds::parseBoolean(theValue)) {
			video.enableOpenGlMode();
		}
	});

	mEngine.registerSpritePropertySetter<ds::ui::GstVideo>(
		"video_nvdecode",
		[](ds::ui::GstVideo& video, const std::string& theValue, const std::string& fileReferrer) {
		video.setNVDecode(ds::parseBoolean(theValue));
	});
}

GstVideoService::~GstVideoService() {}

bool GstVideoService::getValidInstall() { return mValidInstall; }

const std::string& GstVideoService::getErrorMessage() { return mErrorMessage; }

void gstLogFunction(GstDebugCategory* category, GstDebugLevel level, const gchar* file, const gchar* function, gint line,
					GObject* object, GstDebugMessage* message, gpointer user_data) {
	DS_LOG_VERBOSE(3, "GST_DEBUG " << level << " " << file << " " << function << " " << gst_debug_message_get(message));
}

void GstVideoService::start() {
	auto gstVersion = std::string(std::to_string(GST_VERSION_MAJOR) + "." + std::to_string(GST_VERSION_MINOR) + "." +
								  std::to_string(GST_VERSION_MICRO));

#ifdef _WIN32
	// Only need to add Gstreamer bin-path in Windows
	// Add the primary dll's to the PATH variable
	if (!ENV_CHECK.addGStreamerBinPath()) {
		DS_LOG_WARNING("Couldn't find a binary directory for GStreamer! Install GStreamer version " << gstVersion);
		mValidInstall = false;
		return;
	}

#endif
	// Initialization must happen after we know about the bin path, or we'll get endless warnings about missing dll's
	GError* pError;
	int		success = gst_init_check(NULL, NULL, &pError);
	if (success == FALSE) {
		std::stringstream errorStream;
		errorStream << "GStreamerWrapper: failed to initialize GStreamer: " << pError->message;
		std::string errorStr = errorStream.str();
		DS_LOG_ERROR(errorStr);
		mErrorMessage = errorStr;
		mValidInstall = false;
		return;
	}

	// gst_update_registry();

	// Set the plugin path using GStreamer's registry
	DS_LOG_INFO("Initialized GStreamer version " << gstVersion);

	GstRegistry* registery;
	registery = gst_registry_get();

	/* This is another method of setting the plugin path (instead of specifying GST_PLUGIN_PATH), but is a bit slower.
	auto rootPathy = std::getenv("GSTREAMER_1_0_ROOT_MSVC_X86_64");
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

	// gst_registry_load
	auto playbinPlugin = gst_registry_find_plugin(registery, "videoconvert");
	if (playbinPlugin) {
		std::string pluginVersion = gst_plugin_get_version(playbinPlugin);
		if (pluginVersion != gstVersion) {
			DS_LOG_WARNING("Plugin version and compiled GStreamer version don't match. You have version "
						   << pluginVersion << ". If you experience problems, install gstreamer " << gstVersion);
		}
		gst_object_unref(playbinPlugin);
	}

	if (ds::getLogger().hasVerboseLevel(3)) {
		gst_debug_set_active(true);
		gst_debug_set_threshold_from_string("*:3", true);  // 4 for gstreamer is the threshold for a TON of stuff
		gst_debug_add_log_function(*gstLogFunction, NULL, NULL);
	}

	mValidInstall = true;
}


}  // namespace gstreamer
}  // namespace ds
