#include "stdafx.h"

#include "ds/ui/sprite/text_defs.h"

namespace ds {
namespace ui {

// Alignment
Alignment::Enum Alignment::fromString(const std::string& str) {
	if(!str.empty()) {
		if(str[0] == 'R' || str[0] == 'r') return kRight;
		if(str[0] == 'C' || str[0] == 'c') return kCenter;
		if(str[0] == 'J' || str[0] == 'j') return kJustify;
	}
	return Alignment::kLeft;
}



TextStyle::TextStyle(const std::string& font, const std::string& configName, const double size, const double leading,
	const double letterSpacing, const ci::ColorA& c, const ds::ui::Alignment::Enum& alignment)
	: mFont(font)
	, mName(configName)
	, mSize(size)
	, mLeading(leading)
	, mLetterSpacing(letterSpacing)
	, mColor(c)
	, mAlignment(alignment) 
	, mFitMinTextSize(0.0)
	, mFitMaxTextSize(0.0)
{}


TextStyle::TextStyle()
	: mFont("Sans")
	, mName("default")
	, mSize(12.0)
	, mLeading(1.0)
	, mLetterSpacing(0.0)
	, mColor(ci::ColorA::white())
	, mAlignment(Alignment::kLeft)
	, mFitMinTextSize(0.0)
	, mFitMaxTextSize(0.0) 
{}


} // namespace ui
} // namespace ds
