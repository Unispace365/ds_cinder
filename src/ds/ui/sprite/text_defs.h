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
		kCenter,
		kJustify
	};

	Alignment::Enum	fromString(const std::string&);
}


enum class TextWeight : int {
	kThin = 100,
	kUltraLight = 200,
	kLight = 300,
	kSemilight = 350,
	kBook = 380,
	kNormal = 400,
	kMedium = 500,
	kSemibold = 600,
	kBold = 700,
	kUltrabold = 800,
	kHeavy = 900,
	kUltraHeavy = 1000
};

enum class EllipsizeMode : int {
	kEllipsizeNone = 0, // Doesn't add ellipses
	kEllipsizeStart = 1, // Adds ellipses to the beginning of the text if it doesn't fit in the resize limit
	kEllipsizeMiddle = 2, // Adds ellipses to the middle of the text if it doesn't fit in the resize limit
	kEllipsizeEnd = 3 // Adds ellipses to the end of the text if it doesn't fit in the resize limit
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_TEXTDEFS_H
