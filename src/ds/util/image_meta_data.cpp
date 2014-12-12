#include "image_meta_data.h"

#include <intrin.h>
#include <unordered_map>
#include <cinder/ImageIo.h>
#include <cinder/Surface.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include "ds/app/environment.h"
#include "ds/debug/logger.h"
#include "ds/storage/persistent_cache.h"
#include "ds/util/file_meta_data.h"

namespace ds {

namespace {

// Should have universal formats somewhere
const int					FORMAT_UNKNOWN = 0;
const int					FORMAT_PNG = 1;

// Storage object
const std::string			PATH_SZ("q");
const std::string			WIDTH_SZ("w");
const std::string			HEIGHT_SZ("h");
const std::string			TIMESTAMP_SZ("ts");
//PersistentCache				DB("ds/imagemetadata", 1, PersistentCache::FieldList().addString(PATH_SZ).addInt(WIDTH_SZ).addInt(HEIGHT_SZ).addInt(TIMESTAMP_SZ));

int							get_format(const std::string& filename) {
	const Poco::Path		path(filename);
	std::string				ext = path.getExtension();
	Poco::toLowerInPlace(ext);
	if (ext == "png") return FORMAT_PNG;
	return FORMAT_UNKNOWN;
}

bool						is_little_endian() {
	int n = 1;
	return (*(char*)&n == 1);
}

bool						get_format_png(const std::string& filename, ci::Vec2f& outSize) {
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
void						super_slow_image_atts(const std::string& filename, ci::Vec2f& outSize) {
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

	void				add(const std::string& filePath, const ci::Vec2f size){
		if(size.x> 0 && size.y > 0){
			try{
				ImageAtts atts(size);
				atts.mLastModified = Poco::File(filePath).getLastModified();
				mCache[filePath] = atts;
			} catch(std::exception const& ex){
				//HAHAHAHAHAHAHAHA
				DS_LOG_WARNING("ImageAttsCache exception: " << ex.what());
			} 
		}
	}

	ci::Vec2f			getSize(const std::string& fn) {
		// If I've got a cached item and the modified dates match, use that.
		// Note: for the actual path, use the expanded fn.
		const std::string	expanded_fn(ds::Environment::expand(fn));
		try {
			auto f = mCache.find(fn);
			if (f != mCache.end() && f->second.mLastModified == Poco::File(expanded_fn).getLastModified()) {
				return f->second.mSize;
			}
		} catch (std::exception const&) {
		}

		try {
			// Generate the cache:
			ImageAtts		atts = generate(expanded_fn);
			if (atts.mSize.x > 0.0f && atts.mSize.y > 0.0f) {
				atts.mLastModified = Poco::File(expanded_fn).getLastModified();
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
ImageMetaData::ImageMetaData()
		: mSize(0.0f, 0.0f) {
}

ImageMetaData::ImageMetaData(const std::string& filename)
		: mSize(0.0f, 0.0f) {
	mSize = CACHE.getSize(filename);
}

bool ImageMetaData::empty() const {
	return mSize.x < 0.5f || mSize.y < 0.5;
}

void ImageMetaData::add( const std::string& filePath, const ci::Vec2f size ){
	CACHE.add(filePath, size);
}


} // namespace ds