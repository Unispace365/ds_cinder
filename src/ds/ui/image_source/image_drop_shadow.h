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
	/**
	 * \param radius is the radius of each corner of the drop shadow, in pixels.
	 * \param falloff is how quickly the shadow decays.
	 * \param color is the color of the drop shadow.
	 */
	ImageDropShadow(const float radius, const float falloff, const ci::ColorA& color);
	/**
	 * \param radius is the radius of each corner of the drop shadow, in pixels.
	 * \param falloff is how quickly the shadow decays.
	 * \param offset is an offset from 0,0
	 * \param color is the color of the drop shadow.
	 * \param border lets you control the border thickness before the shading
	 * kicks in; at 1 there is no border, you can go up anywhere from there.
	 */
	ImageDropShadow(const float radius, const float falloff,
					const ci::Vec2f& offset, const ci::ColorA& color,
					const float border = 1.0);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEDROPSHADOW_H_
