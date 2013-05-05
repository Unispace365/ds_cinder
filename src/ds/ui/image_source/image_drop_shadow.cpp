#include "image_drop_shadow.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageDropShadow
 */
ImageDropShadow::ImageDropShadow(const float radius, const float falloff)
	: ImageArc(static_cast<int>(radius*2.0f)+1, static_cast<int>(radius*2.0f)+1, "resource:drop_shadow")
{
}

} // namespace ui
} // namespace ds
