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
}

/**
 * \class ds::ui::ImageDropShadow
 */
ImageDropShadow::ImageDropShadow(const float radius, const float falloff, const ci::ColorA& color)
	: ImageArc(static_cast<int>(radius*2.0f)+1, static_cast<int>(radius*2.0f)+1, "resource:drop_shadow")
{
	addColorInput(color);
	addFloatInput(falloff);
}

} // namespace ui
} // namespace ds
