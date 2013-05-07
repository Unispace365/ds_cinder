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

static inline double clip(const double v, const double min, const double max)
{
	if (v <= min) return min;
	if (v >= max) return max;
	return v;
}

static inline uint8_t to_color(const float inv)
{
	if (inv <= 0.0f) return 0;
	if (inv >= 1.0f) return 255;
	return static_cast<uint8_t>(inv*255.0f);
}

bool RenderCircle::on(ci::Surface8u& s, ds::arc::Arc& a)
{
	s.setPremultiplied(false);

	RenderCircleParams	params;
	const double				cenx = (s.getWidth()-1)/2.0,
											ceny = (s.getHeight()-1)/2.0;
	const double				max_dist = ds::math::dist(cenx, ceny, cenx, 0.0);

	auto			pix = s.getIter();
	double		y = 0;
	while (pix.line()) {
		double	x = 0;
		while (pix.pixel()) {
			pix.r() = 0;
			pix.g() = 0;
			pix.b() = 0;
			pix.a() = 0;
			
			const double		dist = ds::math::dist(cenx, ceny, x, y);
			if (dist < max_dist) {
				params.mDist = 1.0 - clip(dist / max_dist, 0.0, 1.0);
				params.mDegree = clip(ds::math::degree(x - cenx, ceny - y) / 360.0, 0.0, 1.0);
				params.mOutput = ci::ColorA(1.0f, 0.0f, 0.0f, 1.0f);

				// render
				a.renderCircle(params);
				if (params.mOutput.a > 0.0f) {
					// antialias
					if (dist > (max_dist - 1.0)) {
						params.mOutput.a *= static_cast<float>(max_dist-dist);
					}
					pix.r() = to_color(params.mOutput.r);
					pix.g() = to_color(params.mOutput.g);
					pix.b() = to_color(params.mOutput.b);
					pix.a() = to_color(params.mOutput.a);
				}
			}

			++x;
		}
		++y;
	}
	return true;
}

} // namespace arc
} // namespace ds
