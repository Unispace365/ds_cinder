#include "stdafx.h"

#include "ds/ui/sprite/fit.h"
#include "ds/util/parse_util.h"

using namespace ci;

namespace ds::ui {

glm::mat3x2 Fit::calcTransform(const Rectf& outer, const Rectf& inner, bool normalized) const {
	// See: https://svgwg.org/svg2-draft/coords.html#ComputingAViewportsTransform
	glm::mat3x2 m32;

	if (inner.getWidth() > 0 && inner.getHeight() > 0) {
		if (mAlign == Align::NONE || mMeetOrSlice != MeetOrSlice::NONE) {
			m32[0][0] = outer.getWidth() / inner.getWidth();   // scale-x
			m32[1][1] = outer.getHeight() / inner.getHeight(); // scale-y
		}
		if (mMeetOrSlice == MeetOrSlice::MEET)						//
			m32[0][0] = m32[1][1] = glm::min(m32[0][0], m32[1][1]); //
		else if (mMeetOrSlice == MeetOrSlice::SLICE)				//
			m32[0][0] = m32[1][1] = glm::max(m32[0][0], m32[1][1]); //
		m32[2][0] = outer.x1 - (inner.x1 * m32[0][0]);				// translate-x
		m32[2][1] = outer.y1 - (inner.y1 * m32[1][1]);				// translate-y

		if (mAlign == Align::X_MID_Y_MIN || mAlign == Align::X_MID_Y_MID || mAlign == Align::X_MID_Y_MAX)
			m32[2][0] += (outer.getWidth() - inner.getWidth() * m32[0][0]) * 0.5f;
		else if (mAlign == Align::X_MAX_Y_MIN || mAlign == Align::X_MAX_Y_MID || mAlign == Align::X_MAX_Y_MAX)
			m32[2][0] += outer.getWidth() - inner.getWidth() * m32[0][0];

		if (mAlign == Align::X_MIN_Y_MID || mAlign == Align::X_MID_Y_MID || mAlign == Align::X_MAX_Y_MID)
			m32[2][1] += (outer.getHeight() - inner.getHeight() * m32[1][1]) * 0.5f;
		else if (mAlign == Align::X_MIN_Y_MAX || mAlign == Align::X_MID_Y_MAX || mAlign == Align::X_MAX_Y_MAX)
			m32[2][1] += outer.getHeight() - inner.getHeight() * m32[1][1];
	} else {
		m32[2][0] = outer.x1;
		m32[2][1] = outer.y1;
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
		mAlign		 = Align::NONE;
		mMeetOrSlice = MeetOrSlice::NONE;
		return;
	}

	if (isSimilar(*sInOut, "xmin", 4)) {
		*sInOut += 4;
		if (!*sInOut) throw std::runtime_error("Fit::parse() - Unexpected end of line");
		if (isSimilar(*sInOut, "ymin", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MIN_Y_MIN;
		} else if (isSimilar(*sInOut, "ymid", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MIN_Y_MID;
		} else if (isSimilar(*sInOut, "ymax", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MIN_Y_MAX;
		}
	} else if (isSimilar(*sInOut, "xmid", 4)) {
		*sInOut += 4;
		if (!*sInOut) throw std::runtime_error("Fit::parse() - Unexpected end of line");
		if (isSimilar(*sInOut, "ymin", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MID_Y_MIN;
		} else if (isSimilar(*sInOut, "ymid", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MID_Y_MID;
		} else if (isSimilar(*sInOut, "ymax", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MID_Y_MAX;
		}
	} else if (isSimilar(*sInOut, "xmax", 4)) {
		*sInOut += 4;
		if (!*sInOut) throw std::runtime_error("Fit::parse() - Unexpected end of line");
		if (isSimilar(*sInOut, "ymin", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MAX_Y_MIN;
		} else if (isSimilar(*sInOut, "ymid", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MAX_Y_MID;
		} else if (isSimilar(*sInOut, "ymax", 4)) {
			*sInOut += 4;
			mAlign = Align::X_MAX_Y_MAX;
		}
	}

	skipSpace(sInOut);

	if (!*sInOut) {
		mMeetOrSlice = MeetOrSlice::NONE;
	} else if (isSimilar(*sInOut, "meet", 4)) {
		*sInOut += 4;
		mMeetOrSlice = MeetOrSlice::MEET;
	} else if (isSimilar(*sInOut, "slice", 5)) {
		*sInOut += 5;
		mMeetOrSlice = MeetOrSlice::SLICE;
	}
}

} // namespace ds::ui
