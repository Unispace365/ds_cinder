#include "touch_mode.h"

#include <boost/algorithm/string.hpp>
#include <ds/cfg/settings.h>

namespace ds {
namespace ui {

bool TouchMode::hasTuio(const TouchMode::Enum &mode) {
	return mode == TouchMode::kTuio || mode == TouchMode::kTuioAndMouse || mode == TouchMode::kAll;
}

bool TouchMode::hasSystem(const TouchMode::Enum &mode) {
	return mode == TouchMode::kSystem || mode == TouchMode::kSystemAndMouse || mode == TouchMode::kAll;
}

bool TouchMode::hasMouse(const TouchMode::Enum &mode) {
	return mode == TouchMode::kTuioAndMouse || mode == TouchMode::kSystemAndMouse || mode == TouchMode::kAll;
}

TouchMode::Enum TouchMode::fromString(const std::string &_str) {
	TouchMode::Enum		mode(TouchMode::kAll);
	std::string			str(_str);
	boost::algorithm::to_lower(str);
	if (str == "tuio") mode = TouchMode::kTuio;
	else if (str == "tuioandmouse") mode = TouchMode::kTuioAndMouse;
	else if (str == "system") mode = TouchMode::kSystem;
	else if (str == "systemandmouse") mode = TouchMode::kSystemAndMouse;
	else if(str == "all") mode = TouchMode::kAll;
	return mode;
}

std::string TouchMode::toString(const TouchMode::Enum &m) {
	if (m == TouchMode::kTuio) return "Tuio";
	if (m == TouchMode::kTuioAndMouse) return "TuioAndMouse";
	if (m == TouchMode::kSystem) return "System";
	if (m == TouchMode::kSystemAndMouse) return "SystemAndMouse";
	if (m == TouchMode::kAll) return "All";
	return "Unknown";
}

TouchMode::Enum TouchMode::fromSettings(const ds::cfg::Settings &s) {
	// Default to tuio and mouse being enabled.
	TouchMode::Enum		mode(TouchMode::kTuioAndMouse);
	// Backwards compatibility
	if (s.getBoolSize("enable_system_multitouch") > 0) {
		const bool		system = s.getBool("enable_system_multitouch", 0, false),
						mouse = s.getBool("enable_mouse_events", 0, true);
		if (system) {
			mode = (mouse ? TouchMode::kSystemAndMouse : TouchMode::kSystem);
		} else {
			mode = (mouse ? TouchMode::kTuioAndMouse : TouchMode::kTuio);
		}
	}
	// The correct way to do this, overrides all other options
	if (s.getTextSize("touch_mode") > 0) {
		mode = TouchMode::fromString(s.getText("touch_mode", 0, ""));
	}
	return mode;
}

TouchMode::Enum TouchMode::next(const TouchMode::Enum &m) {
	if (m == TouchMode::kAll) return kAll;
	if (m == TouchMode::kTuio) return kTuioAndMouse;
	if (m == TouchMode::kTuioAndMouse) return kTuio;
	if (m == TouchMode::kSystem) return kSystemAndMouse;
	if (m == TouchMode::kSystemAndMouse) return kSystem;
	return kTuioAndMouse;
}

} // namespace ui
} // namespace ds
