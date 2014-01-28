#include "image_meta_data.h"

#include <intrin.h>
#include <unordered_map>
#include <cinder/ImageIo.h>
#include <cinder/Surface.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include "ds/util/file_meta_data.h"
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

bool						get_format_png(const std::string& filename, ci::Vec2f& outSize)
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

	outSize.x = static_cast<float>(width);
	outSize.y = static_cast<float>(height);
	return true;
}

// A horrible fallback when no meta info has been supplied about the image size.
void						super_slow_image_atts(const std::string& filename, ci::Vec2f& outSize)
{
	try {
		if (filename.empty()) return;
		DS_LOG_WARNING_M("ImageFileAtts Going to load image synchronously; this will affect performance, filename: " << filename, GENERAL_LOG);
		// Just load the image to get the dimensions -- this will incur what is
		// unnecessarily overhead in one situation (I am in client/server mode),
		// but is otherwise the right thing to do.
		auto s = ci::Surface8u(ci::loadImage(filename));
		if (s) {
			outSize = ci::Vec2f(static_cast<float>(s.getWidth()), static_cast<float>(s.getHeight()));
		}
	} catch (std::exception const& ex) {
		std::cout << "ImageMetaData error loading file (" << filename << ") = " << ex.what() << std::endl;
	}
}

}

// Store a cache of parsed files.  We should have this cache to a database, but for now, just have
// it generate at each run.
namespace {
class ImageAtts {
public:
	ImageAtts() {
	}

	ImageAtts(const ci::Vec2f& size) : mSize(size) {
	}

	Poco::Timestamp		mLastModified;
	ci::Vec2f			mSize;
};

class ImageAttsCache {
public:
	ImageAttsCache() {
	}

	ci::Vec2f			getSize(const std::string& fn) {
		// If I've got a cached item and the modified dates match, use that.
		try {
			auto f = mCache.find(fn);
			if (f != mCache.end() && f->second.mLastModified == Poco::File(fn).getLastModified()) {
				return f->second.mSize;
			}
		} catch (std::exception const&) {
		}

		try {
			// Generate the cache:
			ImageAtts		atts = generate(fn);
			if (atts.mSize.x > 0.0f && atts.mSize.y > 0.0f) {
				atts.mLastModified = Poco::File(fn).getLastModified();
				mCache[fn] = atts;
				return atts.mSize;
			}
		} catch (std::exception const&) {
		}
		return ci::Vec2f(0.0f, 0.0f);
	}

private:
	ImageAtts			generate(const std::string& fn) const {
		// 1. Look for meta data encoded in file name
		try {
			FileMetaData		meta(fn);
			const int			w = meta.findValueType<int>("w", -1),
								h = meta.findValueType<int>("h", -1);
			if (w > 0 && h > 0) {
				return ImageAtts(ci::Vec2f(static_cast<float>(w), static_cast<float>(h)));
			}
		} catch (std::exception const&) {
		}

		// 3. Probe known file formats
		try {
			ImageAtts			atts;
			const int			format = get_format(fn);
			if (format == FORMAT_PNG && get_format_png(fn, atts.mSize)) {
				return atts;
			}
		} catch (std::exception const& e) {
			DS_LOG_WARNING_M("ImageFileAtts() error=" << e.what(), GENERAL_LOG);
		}

		// 4. Load the whole damn image in and get that.
		ImageAtts			atts;
		super_slow_image_atts(fn, atts.mSize);
		return atts;
	}

	std::unordered_map<std::string, ImageAtts>	mCache;
};

ImageAttsCache			CACHE;
}

/**
 * \class ds::ImageMetaData
 */
ImageMetaData::ImageMetaData(const std::string& filename)
		: mSize(0.0f, 0.0f) {
	mSize = CACHE.getSize(filename);
}

} // namespace ds