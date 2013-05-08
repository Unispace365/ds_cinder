#include "file_meta_data.h"

#include <intrin.h>
#include <cinder/ImageIo.h>
#include <cinder/Surface.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include "ds/util/file_name_parser.h"
#include "ds/debug/logger.h"

namespace ds {

namespace {

// Should have universal formats somewhere
const int				FORMAT_UNKNOWN = 0;
const int				FORMAT_PNG = 1;

int							get_format(const std::string& filename)
{
	const Poco::Path	path(filename);
	std::string				ext = path.getExtension();
	Poco::toLowerInPlace(ext);
	if (ext == "png") return FORMAT_PNG;
	return FORMAT_UNKNOWN;
}

bool						is_little_endian()
{
	int n = 1;
	return (*(char*)&n == 1);
}

bool						get_format_png(const std::string& filename, ImageFileAtts& atts)
{
	std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
	if (!file.is_open() || !file) return false;

	// Skip PNG file signature
	file.seekg(8, std::ios_base::cur);

	// First chunk: IHDR image header
	// Skip Chunk Length
	file.seekg(4, std::ios_base::cur);
	// Skip Chunk Type
	file.seekg(4, std::ios_base::cur);

	__int32 width, height;

	file.read((char*)&width, 4);
	file.read((char*)&height, 4);

	// PNG format stores as big endian, convert to little
	if (is_little_endian()) {
		width = _byteswap_ulong(width);
		height = _byteswap_ulong(height);
	}

	atts.mSize.x = static_cast<float>(width);
	atts.mSize.y = static_cast<float>(height);
	return true;
}

// A horrible fallback when no meta info has been supplied about the image size.
void						super_slow_image_atts(const std::string& filename, ImageFileAtts& atts)
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
	// 1. Look for meta data encoded in file name
  try {
    mSize = parseFileMetaDataSize(filename);
		return;
  } catch (ParseFileMetaException &e) {
		DS_LOG_WARNING_M("ImageFileAtts() error=" << e.what(), GENERAL_LOG);
  }

	// 2. Probe known file formats
	try {
		const int		format = get_format(filename);
		if (format == FORMAT_PNG && get_format_png(filename, *this)) return;
	} catch (std::exception const& e) {
		DS_LOG_WARNING_M("ImageFileAtts() error=" << e.what(), GENERAL_LOG);
	}

	// 3. Load the whole damn image in and get that.
	super_slow_image_atts(filename, *this);
}

} // namespace ds