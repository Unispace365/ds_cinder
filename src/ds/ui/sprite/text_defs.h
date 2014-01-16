#pragma once
#ifndef DS_UI_SPRITE_TEXTDEFS_H
#define DS_UI_SPRITE_TEXTDEFS_H

#include <string>

namespace ds {
namespace ui {

namespace Alignment {
	enum Enum {
		kLeft,
		kRight,
		kCenter
	};

	Alignment::Enum	fromString(const std::string&);
}

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_TEXTDEFS_H
