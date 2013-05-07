#include "ds/arc/arc_render_circle.h"

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

bool RenderCircle::on(ci::Surface8u& s, ds::arc::Arc& a)
{
	s.setPremultiplied(false);

	auto pix = s.getIter();
	int y = 0;
	while (pix.line()) {
		int x = 0;
		while (pix.pixel()) {
			pix.r() = 255;
			pix.g() = 0;
			pix.b() = 0;
			pix.a() = 0;
			++x;
		}
	}
	return true;
}

} // namespace arc
} // namespace ds
