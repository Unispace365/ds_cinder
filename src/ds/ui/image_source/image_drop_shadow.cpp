#include "image_drop_shadow.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageDropShadow
 */
ImageDropShadow::ImageDropShadow(const float radius, const float falloff)
	: ImageArc(static_cast<int>(radius*2.0f)+1, static_cast<int>(radius*2.0f)+1, "resource:drop_shadow")
{
	addColorInput(ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));
	addFloatInput(falloff);
	addVec2Input(ci::Vec2d(0.0, 0.0));
}

/**
 * \class ds::ui::ImageDropShadow
 */
ImageDropShadow::ImageDropShadow(const float radius, const float falloff, const ci::ColorA& color)
	: ImageArc(static_cast<int>(radius*2.0f)+1, static_cast<int>(radius*2.0f)+1, "resource:drop_shadow")
{
	addColorInput(color);
	addFloatInput(falloff);
	addVec2Input(ci::Vec2d(0.0, 0.0));
}

/**
 * \class ds::ui::ImageDropShadow
 */
ImageDropShadow::ImageDropShadow(	const float radius,
									const float falloff,
									const ci::Vec2f& offset,
									const ci::ColorA& color,
									const float border)
	: ImageArc(static_cast<int>(radius*2.0f)+1, static_cast<int>(radius*2.0f)+1, "resource:drop_shadow")
{
	addColorInput(color);
	addFloatInput(falloff);
	addVec2Input(ci::Vec2d(offset.x, offset.y));
	addFloatInput(border);
}

} // namespace ui
} // namespace ds
