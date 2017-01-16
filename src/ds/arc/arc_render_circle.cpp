#include "stdafx.h"

#include "ds/arc/arc_render_circle.h"

#include "ds/math/math_func.h"

namespace ds {
namespace arc {

/**
 * ds::arc::RenderCircleParams
 */
RenderCircleParams::RenderCircleParams()
{
}

/**
 * ds::arc::RenderCircle
 */
RenderCircle::RenderCircle()
{
}

static inline float un_premult(const float v, const float a)
{
	if (a < 1.0f && a > 0.0f) {
		return v / a;
	}
	return v;
}

static inline uint8_t to_color(const float inv)
{
	if (inv <= 0.0f) return 0;
	if (inv >= 1.0f) return 255;
	return static_cast<uint8_t>(inv*255.0f);
}

bool RenderCircle::on(const Input& input, ci::Surface8u& s, ds::arc::Arc& a)
{
	s.setPremultiplied(false);

	RenderCircleParams	params;
	params.mW = s.getWidth();
	params.mH = s.getHeight();
	params.mCenX = (s.getWidth()-1)/2.0;
	params.mCenY = (s.getHeight()-1)/2.0;
	params.mMaxDist = ds::math::dist(params.mCenX, params.mCenY, params.mCenX, 0.0);

	auto			pix = s.getIter();
	params.mY = 0.0;
	while (pix.line()) {
		params.mX = 0.0;
		while (pix.pixel()) {
			params.mOutput = ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f);
			a.renderCircle(input, params);
			pix.r() = to_color(un_premult(params.mOutput.r, params.mOutput.a));
			pix.g() = to_color(un_premult(params.mOutput.g, params.mOutput.a));
			pix.b() = to_color(un_premult(params.mOutput.b, params.mOutput.a));
			pix.a() = to_color(params.mOutput.a);

			++params.mX;
		}
		++params.mY;
	}

	return true;
}

} // namespace arc
} // namespace ds
