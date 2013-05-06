#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGESOURCE_H_
#define DS_UI_IMAGESOURCE_IMAGESOURCE_H_

namespace ds {
class DataBuffer;

namespace ui {
class ImageGenerator;
class SpriteEngine;

/**
 * \class ds::ui::ImageSource
 * \brief The public API for setting image source config info. A source
 * basically just stores settings info, and provides access to the
 * ImageGenerator, which is what actually generates the image.
 */
class ImageSource {
public:
	virtual ~ImageSource();

	virtual ImageGenerator*			newGenerator(SpriteEngine&) const = 0;

protected:
	ImageSource();
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGESOURCE_H_
