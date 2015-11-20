#include "app_defs.h"

#include <ds/debug/logger.h>

namespace fullstarter {

namespace {
// _SETTINGS_SETUP
const std::string		_SETTINGS_SETUP("setup");
const std::string		_SETTINGS_DEBUG_ONLY("d-o");
const std::string		_SETTINGS_LAYOUT("layout");


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


// PHYSICS
const int				PHYSICS_INDUSTRIES_LAYER_0_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_1_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_2_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
const int				PHYSICS_INDUSTRIES_LAYER_3_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // !namespace fullstarter