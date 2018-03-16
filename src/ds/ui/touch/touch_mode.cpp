#include "stdafx.h"

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

TouchMode::Enum TouchMode::fromSettings(ds::cfg::Settings &s) {
	// Default to tuio and mouse being enabled.
	TouchMode::Enum		mode(TouchMode::kTuioAndMouse);
	mode = TouchMode::fromString(s.getString("touch:mode"));
	
	return mode;
}

TouchMode::Enum TouchMode::next(const TouchMode::Enum &m) {
	if (m == TouchMode::kAll) return kTuio;
	if (m == TouchMode::kTuio) return kTuioAndMouse;
	if (m == TouchMode::kTuioAndMouse) return kSystem;
	if (m == TouchMode::kSystem) return kSystemAndMouse;
	if (m == TouchMode::kSystemAndMouse) return kAll;
	return kAll;
}

} // namespace ui
} // namespace ds
