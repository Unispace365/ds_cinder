#include "stdafx.h"

#include "ds/ui/grid/value.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/util/parse_util.h"

namespace ds::ui {

Value::Value(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

Value::Value(const char** sInOut) {
	parse(sInOut);
}

float Value::asUser(const Dimensions& dimensions) const {
	switch (mUnit) {
	case PERCENTAGE:
		return mValue * dimensions.percentOf / 100;
	case VIEWPORT_WIDTH:
		return mValue * dimensions.viewportSize.x / 100;
	case VIEWPORT_HEIGHT:
		return mValue * dimensions.viewportSize.y / 100;
	case VIEWPORT_MIN:
		return mValue * std::min(dimensions.viewportSize.x, dimensions.viewportSize.y) / 100;
	case VIEWPORT_MAX:
		return mValue * std::max(dimensions.viewportSize.x, dimensions.viewportSize.y) / 100;
	case UNDEFINED:
	case PIXELS:
	case FLEX:
		break;
	}

	return mValue;
}

float Value::asUser(const ui::Sprite* sprite, Direction direction) const {
	assert(sprite && sprite->getParent());

	Dimensions dim;
	dim.percentOf	 = direction == HORIZONTAL ? sprite->getWidth() : sprite->getHeight();
	dim.viewportSize = {sprite->getEngine().getWorldWidth(), sprite->getEngine().getWorldHeight()};
	return asUser(dim);
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
		} else if (strncmp(*sInOut, "vw", 2) == 0) {
			*sInOut += 2;
			mUnit = VIEWPORT_WIDTH;
		} else if (strncmp(*sInOut, "vh", 2) == 0) {
			*sInOut += 2;
			mUnit = VIEWPORT_HEIGHT;
		} else if (strncmp(*sInOut, "vmin", 4) == 0) {
			*sInOut += 4;
			mUnit = VIEWPORT_MIN;
		} else if (strncmp(*sInOut, "vmax", 4) == 0) {
			*sInOut += 4;
			mUnit = VIEWPORT_MAX;
		}
	}
}

} // namespace ds::ui