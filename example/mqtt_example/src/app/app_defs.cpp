#include "app_defs.h"

#include <ds/debug/logger.h>

namespace example {

namespace {
// _SETTINGS_SETUP
const std::string		_SETTINGS_LAYOUT("layout");

}

const ds::BitMask		APP_LOG = ds::Logger::newModule("app");

// SETTINGS
const std::string&		SETTINGS_LAYOUT = _SETTINGS_LAYOUT;


} // !namespace example

