#include "app_defs.h"

#include <ds/debug/logger.h>

namespace fullstarter {

namespace {
	// _SETTINGS_SETUP
	const std::string _SETTINGS_APP("app_settings");


	// PHYSICS
	const int _PHYSICS_INDUSTRIES_LAYER_0_CATEGORY(1 << 1);
	const int _PHYSICS_INDUSTRIES_LAYER_1_CATEGORY(1 << 2);
	const int _PHYSICS_INDUSTRIES_LAYER_2_CATEGORY(1 << 3);
	const int _PHYSICS_INDUSTRIES_LAYER_3_CATEGORY(1 << 4);
} // namespace

const ds::BitMask APP_LOG = ds::Logger::newModule("app");

// SETTINGS
const std::string& SETTINGS_APP = _SETTINGS_APP;


// PHYSICS
const int PHYSICS_INDUSTRIES_LAYER_0_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
const int PHYSICS_INDUSTRIES_LAYER_1_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
const int PHYSICS_INDUSTRIES_LAYER_2_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
const int PHYSICS_INDUSTRIES_LAYER_3_CATEGORY = _PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // namespace fullstarter