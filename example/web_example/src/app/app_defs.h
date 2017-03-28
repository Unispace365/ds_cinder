#ifndef _WEB_EXAMPLE_APP_APPDEFS_H_
#define _WEB_EXAMPLE_APP_APPDEFS_H_

#include <string>
#include <ds/util/bit_mask.h>

namespace web_example {
extern const ds::BitMask		APP_LOG;

// SETTINGS
extern const std::string&		SETTINGS_SETUP;
extern const std::string&		SETTINGS_DEBUG_ONLY;
extern const std::string&		SETTINGS_LAYOUT;
extern const std::string&		SETTINGS_KEYBOARD;

// FONTS
extern const std::string&		UNIV_LT;
extern const std::string&		UNIV_LT_OB;
extern const std::string&		UNIV_OB;
extern const std::string&		UNIV_ROMAN;
extern const std::string&		UNIV_BOLD;
extern const std::string&		UNIV_BOLD_OB;
extern const std::string&		UNIV_BLACK;
extern const std::string&		UNIV_BLACK_OB;

// PHYSICS
// Front layer: 0, Back layer: highest number
extern const int				PHYSICS_INDUSTRIES_LAYER_0_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_1_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_2_CATEGORY;
extern const int				PHYSICS_INDUSTRIES_LAYER_3_CATEGORY;

} // !namespace web_example

#endif // !_WEB_EXAMPLE_APP_APPDEFS_H_

