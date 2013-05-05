#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEDROPSHADOW_H_
#define DS_UI_IMAGESOURCE_IMAGEDROPSHADOW_H_

#include <string>
#include "ds/ui/image_source/image_arc.h"

namespace ds {
class ImageRegistry;

namespace ui {

/**
 * \class ds::ui::ImageDropShadow
 * \brief A convenience for generating a drop shadow (from an arc).
 */
class ImageDropShadow : public ImageArc
{
public:
  /**
   * \param radius is the radius of each corner of the drop shadow, in pixels.
   * \param falloff is how quickly the shadow decays.
   */
	ImageDropShadow(const float radius, const float falloff);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEDROPSHADOW_H_
