#pragma once

#include <ds/util/bit_mask.h>
#include <string>
#include <vector>

namespace waffles {

/// Wall is a multi-screen array where you'll want things to open where you tap them
/// Single is a smaller area where interfaces open at a set location
typedef enum { kAppModeWall = 0, kAppModeSingle } ScreenAppMode;

// static std::vector<int, ci::vec2> TOUCH_POINTS;

// Media Types
static const std::string& MEDIA_TYPE_DIRECTORY_CMS	 = "content";
static const std::string& MEDIA_TYPE_DIRECTORY_LOCAL = "directory_local";
static const std::string& MEDIA_TYPE_FILE_CMS		 = "media";
static const std::string& MEDIA_TYPE_FILE_LOCAL		 = "file_local";
static const std::string& MEDIA_TYPE_CAPTURE		 = "capture";
static const std::string& MEDIA_TYPE_PRESET			 = "preset";
static const std::string& MEDIA_TYPE_PRESENTATION	 = "presentation";
static const std::string& MEDIA_TYPE_PINBOARD		 = "pinboard";
static const std::string& MEDIA_TYPE_ERROR			 = "error";
static const std::string& VIEW_TYPE_TITLED_MEDIA_VIEWER		= "titled_media_viewer";
static const std::string& VIEW_TYPE_LAUNCHER				= "launcher";
static const std::string& VIEW_TYPE_LAUNCHER_PERSISTANT		= "persistant_launcher";
static const std::string& VIEW_TYPE_CUSTOM_MENU				= "custom_menu";
static const std::string& VIEW_TYPE_SEARCH					= "search";
static const std::string& VIEW_TYPE_SELECT_MEDIA_AMBIENT	= "select_media_ambient";
static const std::string& VIEW_TYPE_SELECT_MEDIA_BACKGROUND = "select_media_background";
static const std::string& VIEW_TYPE_PRESENTATION_CONTROLLER = "presentation_controller";
static const std::string& VIEW_TYPE_FULLSCREEN_CONTROLLER	= "fullscreen_controller";
static const std::string& VIEW_TYPE_NOTE_TAKING				= "note_taking";
static const std::string& VIEW_TYPE_SETTINGS				= "settings";
static const std::string& VIEW_TYPE_DIAGNOSTIC				= "diagnostic_viewer";
static const std::string& VIEW_TYPE_STATE_VIEWER			= "state_viewer";
static const std::string& VIEW_TYPE_ERROR					= "error";
static const std::string& VIEW_TYPE_BASE					= "base";


static const int BACKGROUND_TYPE_NONE		= 0;
static const int BACKGROUND_TYPE_PARTICLES	= 1;
static const int BACKGROUND_TYPE_USER_MEDIA = 2;
static const int BACKGROUND_TYPE_DEFAULT	= 3;


static const int AMBIENT_TYPE_NONE			= 0;
static const int AMBIENT_TYPE_DEFAULT_IMAGE = 1;
static const int AMBIENT_TYPE_DEFAULT_VIDEO = 2;
static const int AMBIENT_TYPE_USER_MEDIA	= 3;


static const int ANIMATE_OFF_SHRINK = 0;
static const int ANIMATE_OFF_FADE	= 1;
static const int ANIMATE_OFF_FALL	= 2;

} // namespace waffles
