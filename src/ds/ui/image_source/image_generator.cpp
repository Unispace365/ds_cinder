#include "stdafx.h"

#include "image_generator.h"

#include "ds/app/app_defs.h"
#include "ds/data/data_buffer.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageGenerator
 */
ImageGenerator::ImageGenerator(const char blobType)
	: mBlobType(blobType)
{
}

ImageGenerator::~ImageGenerator()
{
}

char ImageGenerator::getBlobType() const
{
	return mBlobType;
}

} // namespace ui
} // namespace ds
