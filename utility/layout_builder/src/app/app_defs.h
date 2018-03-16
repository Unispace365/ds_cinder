#ifndef _LAYOUT_BUILDER_APP_APPDEFS_H_
#define _LAYOUT_BUILDER_APP_APPDEFS_H_

#include <string>
#include <ds/util/bit_mask.h>

namespace layout_builder {
extern const ds::BitMask		APP_LOG;

// SETTINGS
extern const std::string&		SETTINGS_SETUP;
extern const std::string&		SETTINGS_DEBUG_ONLY;
extern const std::string&		SETTINGS_LAYOUT;


// PHYSICS
// Front layer: 0, Back layer: highest number
extern const int				PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // !namespace layout_builder

#endif // !_LAYOUT_BUILDER_APP_APPDEFS_H_