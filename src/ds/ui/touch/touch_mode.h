#pragma once
#ifndef DS_UI_TOUCH_TOUCHMODE_H_
#define DS_UI_TOUCH_TOUCHMODE_H_

#include <string>

namespace ds {
namespace cfg {
class Settings;
}

namespace ui {

namespace TouchMode {
	enum Enum {
		kTuio,					// Only TUIO events are handled.
		kTuioAndMouse,			// Both TUIO and mouse events are handled. Default.
		kSystem,				// Only system touch events are handled.
		kSystemAndMouse			// Both system touch events and mouse events are handled. First system touch
								// is assumed to be a mouse event and is discarded.
	};

	bool			hasTuio(const TouchMode::Enum&);
	bool			hasSystem(const TouchMode::Enum&);
	bool			hasMouse(const TouchMode::Enum&);

	TouchMode::Enum	fromString(const std::string&);
	std::string		toString(const TouchMode::Enum&);
	TouchMode::Enum	fromSettings(const ds::cfg::Settings&);
	// Step the touch mode to the next logical one
	TouchMode::Enum	next(const TouchMode::Enum&);
}

} // namespace ui
} // namespace ds

#endif
