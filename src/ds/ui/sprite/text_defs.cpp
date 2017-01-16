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

} // namespace ui
} // namespace ds
