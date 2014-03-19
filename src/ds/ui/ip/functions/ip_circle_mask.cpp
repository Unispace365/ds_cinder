#include <ds/ui/ip/functions/ip_circle_mask.h>

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::CircleMask
 */
CircleMask::CircleMask() {
}

void CircleMask::on(const std::string& parameters, ci::Surface8u& s) const {
	if (!s) return;
	int32_t					w = s.getWidth(), h = s.getHeight();
	if (w < 1 || h < 1) return;

	const ci::Vec2f			cen(static_cast<float>(w)/2.0f, static_cast<float>(h)/2.0f);
	const float				max = (cen.x <= cen.y ? cen.x : cen.y);

	ci::Surface8u::Iter		iter = s.getIter();
	int32_t					y = 0;
	while (iter.line()) {
		int32_t				x = 0;
		while (iter.pixel()) {
			const float		d = cen.distance(ci::Vec2f(static_cast<float>(x), static_cast<float>(y)));
			float			alpha_f = 1.0f;
			if (d > max) {
				alpha_f = 0.0f;
			} else if (d > max - 1.0f) {
				alpha_f = 1.0f-(d-(max-1.0f));
			} 
			int32_t			a = static_cast<int32_t>(static_cast<float>(iter.a()) * alpha_f);
			if (a < 0) a = 0;
			else if (a > 255) a = 255;
			iter.a() = static_cast<uint8_t>(a);
			++x;
		}
		++y;
	}
}

} // namespace ip
} // namespace ui
} // namespace ds
