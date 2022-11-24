#ifndef _PERSPECTIVEPICKING_APP_APPDEFS_H_
#define _PERSPECTIVEPICKING_APP_APPDEFS_H_

#include <ds/util/bit_mask.h>
#include <string>

namespace perspective_picking {
extern const ds::BitMask APP_LOG;

// SETTINGS
extern const std::string& SETTINGS_SETUP;
extern const std::string& SETTINGS_DEBUG_ONLY;
extern const std::string& SETTINGS_LAYOUT;
extern const std::string& SETTINGS_KEYBOARD;

// FONTS
extern const std::string& UNIV_LT;
extern const std::string& UNIV_LT_OB;
extern const std::string& UNIV_OB;
extern const std::string& UNIV_ROMAN;
extern const std::string& UNIV_BOLD;
extern const std::string& UNIV_BOLD_OB;
extern const std::string& UNIV_BLACK;
extern const std::string& UNIV_BLACK_OB;

// PHYSICS
// Front layer: 0, Back layer: highest number
extern const int PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
extern const int PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // namespace perspective_picking

#endif // !_PERSPECTIVEPICKING_APP_APPDEFS_H_
