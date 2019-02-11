#include "stdafx.h"

#include "image_meta_data.h"

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
#include "ds/debug/debug_defines.h"

#include "ds/util/exif_reader.h"

namespace ds {

namespace {

// Should have universal formats somewhere
const int					FORMAT_UNKNOWN = 0;
const int					FORMAT_PNG = 1;
const int					FORMAT_JPG = 2;

// Storage object
const std::string			PATH_SZ("q");
const std::string			WIDTH_SZ("w");
const std::string			HEIGHT_SZ("h");
const std::string			TIMESTAMP_SZ("ts");

int							get_format(const std::string& filename) {
	const Poco::Path		path(filename);
	std::string				ext = path.getExtension();
	Poco::toLowerInPlace(ext);
	if (ext == "png") return FORMAT_PNG;
	if (ext == "jpg" || ext == "jpeg") return FORMAT_JPG;
	return FORMAT_UNKNOWN;
}

uint32_t					big_endian_bytes_to_native( const char* bytes ) {
	const unsigned char* data = (unsigned char*)bytes;
	return (data[3]<<0) | (data[2]<<8) | (data[1]<<16) | (data[0]<<24);
}

bool						get_format_png(const std::string& filename, ci::vec2& outSize) {
	std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
	if (!file.is_open() || !file) return false;

	// Skip PNG file signature
	file.seekg(8, std::ios_base::cur);

	// First chunk: IHDR image header
	// Skip Chunk Length
	file.seekg(4, std::ios_base::cur);
	// Skip Chunk Type
	file.seekg(4, std::ios_base::cur);

	char widthBytes[4];
	char heightBytes[4];
	file.read(widthBytes, 4);
	file.read(heightBytes, 4);

	// PNG format stores as big endian, convert to native endianness
	uint32_t width = big_endian_bytes_to_native(widthBytes);
	uint32_t height = big_endian_bytes_to_native(heightBytes);

	// check to make sure we correctly read the size. There's some bad png's out there
	if(width < 1 || width > 20000 || height < 1 || height > 20000){
		return false;
	}

	outSize.x = static_cast<float>(width);
	outSize.y = static_cast<float>(height);
	return true;
}

bool						get_format_jpg(const std::string& filename, ci::vec2& outSize) {
	std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
	if (!file.is_open() || !file) return false;

	char buf[4] = {0, 0, 0, 0};
	unsigned char* ubuf = (unsigned char*)buf;
	unsigned char marker;

	// Read until we find a marker
	const auto nextMarker = [&file, &buf, &ubuf, &marker]() {
		int discardedBytes = 0;
		file.read(buf, 1);
		while (ubuf[0] != 0xFF) {
			discardedBytes++;
			file.read(buf, 1);
		}
		do {
			file.read(buf, 1);
		} while (ubuf[0] == 0xFF);
		marker = ubuf[0];
		return discardedBytes;
	};

	const auto readWord = [&file, &buf]() -> unsigned {
		file.read(buf, 2);
		return ((unsigned char)(buf[0]) << 8) + (unsigned char)(buf[1]);
	};

	// Check Header
	unsigned header;
	if ((header = readWord()) != 0x0000ffd8)
		return false;

	// Read until we find a SOF (Start of Frame) marker
	while(!file.eof()) {
		if (nextMarker() != 0)
			return false;

		if ( ((marker & 0xf0) == 0xc0)
		  && ((marker & 0x0f) != 0x04)
		  && ((marker & 0x0f) != 0x08)
		  && ((marker & 0x0f) != 0x0c))
		{
			// Skip length, precision bytes
			file.seekg(3, std::ios_base::cur);
			const unsigned height = readWord();
			const unsigned width = readWord();
			// Sanity check dimensions
			if (width < 1 || width > 20000 || height < 1 || height > 20000) {
				return false;
			}
			outSize.x = static_cast<float>(width);
			outSize.y = static_cast<float>(height);
			return true;
		}
		else if (marker == 0xDA || marker == 0xD9) {
			return false;
		}
		else {
			// Skip this segment
			auto length = readWord();
			if (length < 2)
				return false;
			file.seekg(length-2, std::ios_base::cur);
		}
	}

	return false;
}


// A horrible fallback when no meta info has been supplied about the image size.
void						super_slow_image_atts(const std::string& filename, ci::vec2& outSize) {
	try {
		if (filename.empty()) return;
		// Just load the image to get the dimensions -- this will incur what is
		// unnecessarily overhead in one situation (I am in client/server mode),
		// but is otherwise the right thing to do.
		const Poco::File file(filename);

		if(!ds::safeFileExistsCheck(filename)){
			DS_LOG_WARNING_M("ImageFileAtts: image file does not exist, filename: " << filename, GENERAL_LOG);
			return;
		}

		DS_LOG_WARNING_M("ImageFileAtts Going to load image synchronously; this will affect performance, filename: " << filename, GENERAL_LOG);

		auto s = ci::Surface8u(ci::loadImage(filename));
		if(s.getData()) {
			outSize = ci::vec2(static_cast<float>(s.getWidth()), static_cast<float>(s.getHeight()));
		} else {
			DS_LOG_WARNING_M("super_slow_image_atts: file could not be loaded, filename: " << filename, GENERAL_LOG);
			outSize = ci::vec2();
		}
	} catch (std::exception const& ex) {
		bool errored = true;

		// try to load it from the web
		try {
			auto s = ci::Surface8u(ci::loadImage(ci::loadUrl(filename)));
			if(s.getData()) {
				outSize = ci::vec2(static_cast<float>(s.getWidth()), static_cast<float>(s.getHeight()));
				errored = false;
			} else {
				DS_LOG_WARNING_M("super_slow_image_atts: file could not be loaded, filename: " << filename, GENERAL_LOG);
				outSize = ci::vec2();
			}
		} catch(ci::StreamExc&){
			DS_LOG_WARNING_M("ImageMetaData stream exception loading file from url (" << filename << ")", GENERAL_LOG);
			
		} catch(std::exception const& extwo){
			if(extwo.what()) {
				DS_LOG_WARNING_M("ImageMetaData error loading file from url (" << filename << ") = " << extwo.what(), GENERAL_LOG);
			} else {
				DS_LOG_WARNING_M("ImageMetaData error loading file from url (" << filename << ")", GENERAL_LOG);
			}
		}

		if(errored){
			if(ex.what()) {
				DS_LOG_WARNING_M("ImageMetaData error loading file (" << filename << ") = " << ex.what(), GENERAL_LOG);
			} else {
				DS_LOG_WARNING_M("ImageMetaData error loading file (" << filename << ")", GENERAL_LOG);
			}
		}
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

	ImageAtts(const ci::vec2& size) : mSize(size) {
	}

	Poco::Timestamp		mLastModified;
	ci::vec2			mSize;
};

class ImageAttsCache {
public:
	ImageAttsCache() {
	}

	void clear() {
		mCache.clear();
	}

	void add(const std::string& filePath, const ci::vec2 size){
		if(size.x> 0 && size.y > 0){
			try{
				ImageAtts atts(size);
				if(ds::safeFileExistsCheck(filePath, false)) {
					const auto file = Poco::File(filePath);
					atts.mLastModified = file.getLastModified();
					mCache[filePath] = atts;
				} else {
					DS_LOG_WARNING_M("ImageAttsCache::add : File does not exist when finding metadata: " << filePath, GENERAL_LOG);
				}
			} catch (std::exception const& ex){
				DS_LOG_WARNING("ImageAttsCache exception: " << ex.what());
			}
		}
	}

	ci::vec2 getSize(const std::string& fn) {
		// If I've got a cached item and the modified dates match, use that.
		// Note: for the actual path, use the expanded fn.

		std::string	expanded_fn;
		bool webMode = false;
		if(fn.find("http") == 0){
			webMode = true;
			expanded_fn = fn;
		} else {
			expanded_fn = ds::Environment::expand(fn);
		}

		try {
			auto f = mCache.find(fn);
			
			if(f != mCache.end()){
				// we hope that the remote image hasn't changed since we grabbed it's size.
				if(webMode){
					return f->second.mSize;
				} else if(f->second.mLastModified == Poco::File(expanded_fn).getLastModified()) {
					return f->second.mSize;
				}
			}
		} catch (std::exception const&) {
		}

		try {
			// Generate the cache:
			ImageAtts		atts = generate(expanded_fn);
			if (atts.mSize.x > 0.0f && atts.mSize.y > 0.0f) {
				// calling anything on an invalid file throws an exception, and web stuff is invalid
				if(!webMode) atts.mLastModified = Poco::File(expanded_fn).getLastModified();
				mCache[fn] = atts;
				return atts.mSize;
			}
		} catch (std::exception const&) {
		}
		return ci::vec2(0.0f, 0.0f);
	}

private:
	ImageAtts			generate(const std::string& fn) const {
		// 1. Look for meta data encoded in file name
		try {
			FileMetaData		meta(fn);
			const int			w = meta.findValueType<int>("w", -1),
								h = meta.findValueType<int>("h", -1);
			if(w > 0 && h > 0) {
				DS_LOG_VERBOSE(7, "ImageAttsCache got filename image size " << w << "x" << h << " for " << fn);
				return ImageAtts(ci::vec2(static_cast<float>(w), static_cast<float>(h)));
			}
		} catch (std::exception const&) {
		}

		// 2. Probe known file formats
		try {
			ImageAtts			atts;
			const int			format = get_format(fn);
			if(format == FORMAT_PNG && get_format_png(fn, atts.mSize)) {
				DS_LOG_VERBOSE(7, "ImageAttsCache got png image size " << atts.mSize.x<< "x" << atts.mSize.y << " for " << fn);
				return atts;
			}
			if(format == FORMAT_JPG && get_format_jpg(fn, atts.mSize)) {
				DS_LOG_VERBOSE(7, "ImageAttsCache got jpg image size " << atts.mSize.x << "x" << atts.mSize.y << " for " << fn);
				return atts;
			}
		} catch (std::exception const& e) {
			DS_LOG_WARNING_M("ImageFileAtts() error=" << e.what(), GENERAL_LOG);
		}

		// 3. let's see if there's exif data
		int outW = 0;
		int outH = 0;
		if(ds::ExifHelper::getImageSize(fn, outW, outH)){
			DS_LOG_VERBOSE(7, "ImageAttsCache got exif image size " << outW << "x" << outH << " for " << fn);
			return ImageAtts(ci::vec2(static_cast<float>(outW), static_cast<float>(outH)));
		}

		// 4. Load the whole damn image in and get that.
		ImageAtts			atts;
		super_slow_image_atts(fn, atts.mSize);
		DS_LOG_VERBOSE(7, "ImageAttsCache got super slow image size " << atts.mSize.x << "x" << atts.mSize.y << " for " << fn);
		return atts;
	}

	std::unordered_map<std::string, ImageAtts>	mCache;
};

ImageAttsCache			CACHE;
}

/**
 * \class ImageMetaData
 */
ImageMetaData::ImageMetaData()
		: mSize(0.0f, 0.0f) {
}

ImageMetaData::ImageMetaData(const std::string& filename)
		: mSize(0.0f, 0.0f) {
	mSize = CACHE.getSize(filename);
}

void ImageMetaData::clearMetadataCache() {
	CACHE.clear();
}

bool ImageMetaData::empty() const {
	return mSize.x < 0.5f || mSize.y < 0.5;
}

void ImageMetaData::add( const std::string& filePath, const ci::vec2 size ){
	CACHE.add(filePath, size);
}


} // namespace ds
