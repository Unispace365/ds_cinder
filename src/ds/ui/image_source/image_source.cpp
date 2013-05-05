#include "image_source.h"

#include "ds/app/app_defs.h"
#include "ds/data/data_buffer.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageSource
 */
ImageSource::ImageSource(const char blobType)
	: mBlobType(blobType)
{
}

ImageSource::~ImageSource()
{
}

char ImageSource::getBlobType() const
{
	return mBlobType;
}

} // namespace ui
} // namespace ds
