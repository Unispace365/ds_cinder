#pragma once
#ifndef DS_UTIL_FILEMETADATA_H_
#define DS_UTIL_FILEMETADATA_H_

#include <string>
#include <cinder/Vector.h>

namespace ds {

/**
 * \class ds::ImageFileAtts
 * \brief Read meta data for image files.
 * NOTE: This can be VERY slow, if the image needs to be loaded.
 */
class ImageFileAtts {
public:
	ImageFileAtts(const std::string& filename);

	ci::Vec2f					mSize;
};

}

#endif // DS_UTIL_FILEMETADATA_H_
