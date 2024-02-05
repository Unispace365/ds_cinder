#include "stdafx.h"

#include "ds/ui/sprite/fit.h"

using namespace ci;

namespace ds::ui {

glm::mat3x2 Fit::calcTransform(const Rectf& outer, const Rectf& inner, bool normalized) const {
	// See: https://svgwg.org/svg2-draft/coords.html#ComputingAViewportsTransform
	glm::mat3x2 m32;

	if (mAlignX == NONE || mAlignY == NONE) return m32;

	if (inner.getWidth() > 0 && inner.getHeight() > 0) {
		m32[0][0] = outer.getWidth() / inner.getWidth();			// scale-x
		m32[1][1] = outer.getHeight() / inner.getHeight();			// scale-y
		if (mMeetOrSlice == MEET)									//
			m32[0][0] = m32[1][1] = glm::min(m32[0][0], m32[1][1]); //
		else if (mMeetOrSlice == SLICE)								//
			m32[0][0] = m32[1][1] = glm::max(m32[0][0], m32[1][1]); //
		m32[2][0] = outer.x1 - (inner.x1 * m32[0][0]);				// translate-x
		m32[2][1] = outer.y1 - (inner.y1 * m32[1][1]);				// translate-y

		if (mAlignX == MID)
			m32[2][0] += (outer.getWidth() - inner.getWidth() * m32[0][0]) * 0.5f;
		else if (mAlignX == MAX)
			m32[2][0] += outer.getWidth() - inner.getWidth() * m32[0][0];
		if (mAlignY == MID)
			m32[2][1] += (outer.getHeight() - inner.getHeight() * m32[1][1]) * 0.5f;
		else if (mAlignY == MAX)
			m32[2][1] += outer.getHeight() - inner.getHeight() * m32[1][1];
	}

	// Normalize.
	if (normalized && outer.getWidth() > 0 && outer.getHeight() > 0) {
		m32[0][0] = inner.getWidth() * m32[0][0] / outer.getWidth();
		m32[1][1] = inner.getHeight() * m32[1][1] / outer.getHeight();
		m32[2][0] /= outer.getWidth();
		m32[2][1] /= outer.getHeight();
	}

	return m32;
}

mat3 Fit::calcTransform3x3(const Rectf& outer, const Rectf& inner, bool normalized) const {
	const auto m32 = calcTransform(outer, inner, normalized);
	return {m32[0][0], m32[0][1], 0.f, m32[1][0], m32[1][1], 0.f, m32[2][0], m32[2][1], 1.f};
}

mat4 Fit::calcTransform4x4(const Rectf& outer, const Rectf& inner, bool normalized) const {
	const auto m32 = calcTransform(outer, inner, normalized);
	return {m32[0][0], m32[0][1], 0.f,	0.f, m32[1][0], m32[1][1], 0.f, 0.f,
			0.f,	   0.f,		  1.0f, 0.f, m32[2][0], m32[2][1], 0.f, 1.f};
}

void Fit::parse(const char** sInOut) {
	skipSpace(sInOut);

	if (!*sInOut) return;

	if (isSimilar(*sInOut, "none", 4)) {
		*sInOut += 4;
		mAlignX = NONE;
		mAlignY = NONE;
		mMeetOrSlice = MEET;
		return;
	}

	if (isSimilar(*sInOut, "xmin", 4)) {
		*sInOut += 4;
		mAlignX = MIN;
	} else if (isSimilar(*sInOut, "xmid", 4)) {
		*sInOut += 4;
		mAlignX = MID;
	} else if (isSimilar(*sInOut, "xmax", 4)) {
		*sInOut += 4;
		mAlignX = MAX;
	}

	if (!*sInOut) return;

	if (isSimilar(*sInOut, "ymin", 4)) {
		*sInOut += 4;
		mAlignY = MIN;
	} else if (isSimilar(*sInOut, "ymid", 4)) {
		*sInOut += 4;
		mAlignY = MID;
	} else if (isSimilar(*sInOut, "ymax", 4)) {
		*sInOut += 4;
		mAlignY = MAX;
	}

	skipSpace(sInOut);

	if (!*sInOut) return;

	if (isSimilar(*sInOut, "meet", 4)) {
		*sInOut += 4;
		mMeetOrSlice = MEET;
	} else if (isSimilar(*sInOut, "slice", 5)) {
		*sInOut += 5;
		mMeetOrSlice = SLICE;
	}
}

} // namespace ds::ui
