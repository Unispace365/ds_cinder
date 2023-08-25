#pragma once
#ifndef DS_UTIL_IMAGEMETADATA_H_
#define DS_UTIL_IMAGEMETADATA_H_

#include <cinder/Vector.h>
#include <string>

namespace ds {

/**
 * \class ImageMetaData
 * \brief Read meta data for image files.
 * NOTE: This can be VERY slow, if the image needs to be loaded.
 */
class ImageMetaData {
  public:
	ImageMetaData();
	ImageMetaData(const std::string& filename);

	/// Clears any stored w/h info
	static void clearMetadataCache();

	bool empty() const;
	void add(const std::string& filePath, const ci::vec2 size);

	ci::vec2 mSize;
};

} // namespace ds

#endif // DS_UTIL_IMAGEMETADATA_H_
