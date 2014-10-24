#include "app_defs.h"

#include <ds/debug/logger.h>

namespace importer_example {

namespace {
// _SETTINGS_SETUP
const std::string		_SETTINGS_SETUP("setup");
const std::string		_SETTINGS_DEBUG_ONLY("d-o");
const std::string		_SETTINGS_LAYOUT("layout");
const std::string		_SETTINGS_KEYBOARD("key");
const std::string		_SETTINGS_ANIMATION("animation");

// FONTS
const std::string		_HELVETICA_NEUE_LT("helvetica-neue-lt");
const std::string		_HELVETICA_NEUE_TH("helvetica-neue-th");
const std::string		_HELVETICA_NEUE_ULTLT("helvetica-neue-ultlt");

// Actual Fonts
const std::string&		_UNIV_LT("");
const std::string&		_UNIV_LT_OB("");
const std::string&		_UNIV_OB("");
const std::string&		_UNIV_ROMAN("");
const std::string&		_UNIV_BOLD("");
const std::string&		_UNIV_BOLD_OB("");
const std::string&		_UNIV_BLACK("");
const std::string&		_UNIV_BLACK_OB("");

// PHYSICS
const int				_PHYSICS_INDUSTRIES_LAYER_0_CATEGORY(1<<1);
const int				_PHYSICS_INDUSTRIES_LAYER_1_CATEGORY(1<<2);
const int				_PHYSICS_INDUSTRIES_LAYER_2_CATEGORY(1<<3);
const int				_PHYSICS_INDUSTRIES_LAYER_3_CATEGORY(1<<4);
}

const ds::BitMask		APP_LOG = ds::Logger::newModule("app");

// SETTINGS
const std::string&		SETTINGS_SETUP = _SETTINGS_SETUP;
const std::string&		SETTINGS_DEBUG_ONLY = _SETTINGS_DEBUG_ONLY;
const std::string&		SETTINGS_LAYOUT = _SETTINGS_LAYOUT;
const std::string&		SETTINGS_KEYBOARD = _SETTINGS_KEYBOARD;

// ACTUAL FONTS
const std::string&		UNIV_LT =		_UNIV_LT;
const std::string&		UNIV_LT_OB =	_UNIV_LT_OB;
const std::string&		UNIV_OB =		_UNIV_OB;
const std::string&		UNIV_ROMAN =	_UNIV_ROMAN;
const std::string&		UNIV_BOLD =		_UNIV_BOLD;
const std::string&		UNIV_BOLD_OB =  _UNIV_BOLD_OB;
const std::string&		UNIV_BLACK =	_UNIV_BLACK;
const std::string&		UNIV_BLACK_OB = _UNIV_BLACK_OB;


// PHYSICS
const int				PHYSICS_INDUSTRIES_LAYER_0_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_1_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_2_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_3_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // !namespace importer_example