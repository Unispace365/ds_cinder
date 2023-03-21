#ifndef _MEDIAVIEWER_APP_APPDEFS_H_
#define _MEDIAVIEWER_APP_APPDEFS_H_

#include <ds/util/bit_mask.h>
#include <string>

namespace mv {
extern const ds::BitMask APP_LOG;

// SETTINGS
extern const std::string& SETTINGS_SETUP;
extern const std::string& SETTINGS_DEBUG_ONLY;
extern const std::string& SETTINGS_LAYOUT;


// PHYSICS
// Front layer: 0, Back layer: highest number
extern const int PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // namespace mv

#endif // !_MEDIAVIEWER_APP_APPDEFS_H_
