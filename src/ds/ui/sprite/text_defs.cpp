#include "ds/ui/sprite/text_defs.h"

namespace ds {
namespace ui {

// Alignment
Alignment::Enum Alignment::fromString(const std::string& str) {
	if (!str.empty()) {
		if (str[0] == 'R' || str[0] == 'r') return kRight;
		if (str[0] == 'C' || str[0] == 'c') return kCenter;
	}
	return Alignment::kLeft;
}

} // namespace ui
} // namespace ds
