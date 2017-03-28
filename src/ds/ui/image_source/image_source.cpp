#include "stdafx.h"

#include "image_source.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageSource
 */
ImageSource::ImageSource() {
}

ImageSource::~ImageSource() {
}

bool ImageSource::generatorMatches(const ImageGenerator&) const {
	return false;
}

} // namespace ui
} // namespace ds
