#include "file_meta_data.h"

#include <cinder/ImageIo.h>
#include <cinder/Surface.h>
#include "ds/util/file_name_parser.h"
#include "ds/debug/logger.h"

namespace ds {

namespace {

void super_slow_image_atts(const std::string& filename, ImageFileAtts& atts)
{
	try {
		if (filename.empty()) return;
		DS_LOG_WARNING_M("Going to load image synchronously; this will affect performance", GENERAL_LOG);
		// Just load the image to get the dimensions -- this will incur what is
		// unnecessarily overhead in one situation (I am in client/server mode),
		// but is otherwise the right thing to do.
		auto s = ci::Surface8u(ci::loadImage(filename));
		if (s) {
			atts.mSize = ci::Vec2f(static_cast<float>(s.getWidth()), static_cast<float>(s.getHeight()));
		}
	} catch (std::exception const&) {
	}
}

}

/**
 * \class ds::ImageFileAtts
 */
ImageFileAtts::ImageFileAtts(const std::string& filename)
	: mSize(0.0f, 0.0f)
{
  try {
    mSize = parseFileMetaDataSize(filename);
  } catch (ParseFileMetaException &e) {
    DS_LOG_WARNING_M("ImageFileAtts() error=" << e.what(), GENERAL_LOG);
    super_slow_image_atts(filename, *this);
  }
}

} // namespace ds