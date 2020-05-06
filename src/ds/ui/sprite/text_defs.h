#pragma once

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

enum class EllipsizeMode : int {
	kEllipsizeNone = 0, // Doesn't add ellipses
	kEllipsizeStart = 1, // Adds ellipses to the beginning of the text if it doesn't fit in the resize limit
	kEllipsizeMiddle = 2, // Adds ellipses to the middle of the text if it doesn't fit in the resize limit
	kEllipsizeEnd = 3 // Adds ellipses to the end of the text if it doesn't fit in the resize limit
};


enum class WrapMode : int {
	kWrapModeWord = 0, // Wraps only on word boundaries
	kWrapModeChar = 1, // Wraps on each character
	kWrapModeOff = 2, //Turns of word wrap
	kWrapModeWordChar  // Wraps on words, and falls back to char if it doesn't fit 
};


/**
* \class TextStyle
* Store config settings for a text style
*/
struct TextStyle{
	TextStyle();
	TextStyle(const std::string& font, const std::string& configName, const double size, const double leading,
		const double letterSpacing, const ci::ColorA&, const ds::ui::Alignment::Enum& = ds::ui::Alignment::kLeft);

	std::string				mName;
	std::string				mFont;
	double					mSize;
	std::vector<double>		mFitSizes;
	double					mFitMaxTextSize;
	double					mFitMinTextSize;
	double					mLeading;
	double					mLetterSpacing;
	ci::ColorA	  			mColor;
	ds::ui::Alignment::Enum	mAlignment;
};

} // namespace ui
} // namespace ds
