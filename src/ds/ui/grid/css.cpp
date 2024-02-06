#include "stdafx.h"

#include <ds/ui/grid/css.h>
#include <ds/ui/sprite/sprite.h>

namespace ds::css {

Value::Value(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

Value::Value(const char** sInOut) {
	parse(sInOut);
}

float Value::asUser(float percentOf) const {
	switch (mUnit) {
	case PIXELS:
		return mValue;
	case PERCENTAGE:
		return mValue * percentOf / 100;
	default:
		throw std::runtime_error("Value can't be calculated");
	}
}

float Value::asUser(const ui::Sprite* sprite, Direction direction) const {
	assert(sprite && sprite->getParent());
	const float percentOf = direction == HORIZONTAL ? sprite->getParent()->getWidth() : sprite->getParent()->getHeight();
	return asUser(percentOf);
}

void Value::parse(const char** sInOut) {
	const char* from = *sInOut;

	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		mUnit = UNDEFINED;
	} else {
		mValue = parseFloat(sInOut);
		if (mValue < 0) throw std::runtime_error("Value must be equal to or greater than 0");

		if (!**sInOut || std::isspace(**sInOut)) {
			DS_LOG_WARNING("No unit given, pixels assumed: " << std::string(from, *sInOut - from).c_str());
			mUnit = PIXELS;
		}

		else if (**sInOut == '%') {
			*sInOut += 1;
			mUnit = PERCENTAGE;
		} else if (strncmp(*sInOut, "px", 2) == 0) {
			*sInOut += 2;
			mUnit = PIXELS;
		} else if (strncmp(*sInOut, "fr", 2) == 0) {
			*sInOut += 2;
			mUnit = FLEX;
		}
	}
}

} // namespace ds::css