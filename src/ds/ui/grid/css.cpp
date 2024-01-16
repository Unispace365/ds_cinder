#include "stdafx.h"

#include <ds/ui/grid/css.h>

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
		throw ValueException(*this, "Value can't be calculated");
	}
}

void Value::parse(const char** sInOut) {
	const char* from = *sInOut;

	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		mUnit = UNDEFINED;
	} else {
		mValue = parseFloat(sInOut);
		if (!(mValue > 0)) throw ParseException(from, "Value must be greater than 0");

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

SizingFn::SizingFn(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

SizingFn::SizingFn(const char** sInOut) {
	parse(sInOut);
}

void SizingFn::parse(const char** sInOut) {
	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		mUnit = UNDEFINED;
	} else if (strncmp(*sInOut, "min-content", 11) == 0) {
		*sInOut += 11;
		mUnit = MIN_CONTENT;
	} else if (strncmp(*sInOut, "max-content", 11) == 0) {
		*sInOut += 11;
		mUnit = MAX_CONTENT;
	} else {
		mValue = Value(sInOut);
		mUnit  = FIXED;
	}
}

void GridLine::parse(const char** sInOut) {
	mIdent.clear();
	mUnit = AUTO;

	skipSpace(sInOut);
	if (strncmp(*sInOut, "auto", 4) == 0) {
		*sInOut += 4;
		return;
	}
	if (strncmp(*sInOut, "span", 4) == 0) {
		*sInOut += 4;
		skipSpace(sInOut);
		if (isNumeric(**sInOut))
			mValue = int(parseFloat(sInOut));
		else if (**sInOut == '[') {
			mIdent = fetchUntil(sInOut, ']');
		}
		mUnit = SPAN;
		return;
	}
	if (**sInOut == '[') {
		mIdent = fetchUntil(sInOut, ']');
		mUnit  = SINGLE;
	} else if (isNumeric(**sInOut)) {
		mValue = int(parseFloat(sInOut));
		mUnit  = SINGLE;
	}
}

PreserveAspectRatio::PreserveAspectRatio(const std::string& str) {
	const char* sInOut = str.c_str();
	parse(&sInOut);
}

PreserveAspectRatio::PreserveAspectRatio(const char** sInOut) {
	parse(sInOut);
}

glm::mat3x2 PreserveAspectRatio::calcTransform(const ci::Rectf& element, const ci::Rectf& viewBox,
											   bool normalized) const {
	// See: https://svgwg.org/svg2-draft/coords.html#ComputingAViewportsTransform
	glm::mat3x2 m32;

	if (viewBox.getWidth() > 0 && viewBox.getHeight() > 0) {
		m32[0][0] = element.getWidth() / viewBox.getWidth();		// scale-x
		m32[1][1] = element.getHeight() / viewBox.getHeight();		// scale-y
		if (align != ALIGN_NONE && meetOrSlice == MEET)				//
			m32[0][0] = m32[1][1] = glm::min(m32[0][0], m32[1][1]); //
		else if (align != ALIGN_NONE && meetOrSlice == SLICE)		//
			m32[0][0] = m32[1][1] = glm::max(m32[0][0], m32[1][1]); //
		m32[2][0] = element.x1 - (viewBox.x1 * m32[0][0]);			// translate-x
		m32[2][1] = element.y1 - (viewBox.y1 * m32[1][1]);			// translate-y

		if (align == ALIGN_X_MID_Y_MIN || align == ALIGN_X_MID_Y_MID || align == ALIGN_X_MID_Y_MAX)
			m32[2][0] += (element.getWidth() - viewBox.getWidth() * m32[0][0]) * 0.5f;
		else if (align == ALIGN_X_MAX_Y_MIN || align == ALIGN_X_MAX_Y_MID || align == ALIGN_X_MAX_Y_MAX)
			m32[2][0] += element.getWidth() - viewBox.getWidth() * m32[0][0];
		if (align == ALIGN_X_MIN_Y_MID || align == ALIGN_X_MID_Y_MID || align == ALIGN_X_MAX_Y_MID)
			m32[2][1] += (element.getHeight() - viewBox.getHeight() * m32[1][1]) * 0.5f;
		else if (align == ALIGN_X_MIN_Y_MAX || align == ALIGN_X_MID_Y_MAX || align == ALIGN_X_MAX_Y_MAX)
			m32[2][1] += element.getHeight() - viewBox.getHeight() * m32[1][1];
	}

	// Normalize.
	if (normalized && element.getWidth() > 0 && element.getHeight() > 0) {
		m32[0][0] = float(viewBox.getWidth()) * m32[0][0] / element.getWidth();
		m32[1][1] = float(viewBox.getHeight()) * m32[1][1] / element.getHeight();
		m32[2][0] /= element.getWidth();
		m32[2][1] /= element.getHeight();
	}

	return m32;
}

void PreserveAspectRatio::parse(const char** sInOut) {
	skipSpace(sInOut);
	auto a = fetchWord(sInOut);

	if (!ci::asciiCaseCmp(a.c_str(), "none"))
		align = ALIGN_NONE;
	else if (!ci::asciiCaseCmp(a.c_str(), "xminymin"))
		align = ALIGN_X_MIN_Y_MIN;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmidymin"))
		align = ALIGN_X_MID_Y_MIN;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmaxymin"))
		align = ALIGN_X_MAX_Y_MIN;
	else if (!ci::asciiCaseCmp(a.c_str(), "xminymid"))
		align = ALIGN_X_MIN_Y_MID;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmidymid"))
		align = ALIGN_X_MID_Y_MID;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmaxymid"))
		align = ALIGN_X_MAX_Y_MID;
	else if (!ci::asciiCaseCmp(a.c_str(), "xminymax"))
		align = ALIGN_X_MIN_Y_MIN;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmidymax"))
		align = ALIGN_X_MID_Y_MAX;
	else if (!ci::asciiCaseCmp(a.c_str(), "xmaxymax"))
		align = ALIGN_X_MAX_Y_MAX;

	skipSpace(sInOut);
	auto m = fetchWord(sInOut);

	if (!ci::asciiCaseCmp(m.c_str(), "meet"))
		meetOrSlice = MEET;
	else if (!ci::asciiCaseCmp(m.c_str(), "slice"))
		meetOrSlice = SLICE;
}


} // namespace ds::css