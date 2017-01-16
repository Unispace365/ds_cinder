#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEGENERATOR_H_
#define DS_UI_IMAGESOURCE_IMAGEGENERATOR_H_

#include <cinder/gl/Texture.h>
#include "ds/util/image_meta_data.h"

namespace ds {
class DataBuffer;

namespace ui {

/**
 * \class ds::ui::ImageGenerator
 * \brief The generator is responsible for creating the actual image.
 * It's intended to be an internal class, hidden from the public API.
 */
class ImageGenerator {
public:
	virtual ~ImageGenerator();

	// Answer meta data about this image.
	virtual bool						getMetaData(ImageMetaData&) const = 0;
	virtual const ci::gl::TextureRef	getImage() = 0;

	char								getBlobType() const;
	virtual void						writeTo(DataBuffer&) const = 0;
	virtual bool						readFrom(DataBuffer&) = 0;

	virtual std::string					getImageFilename() const = 0;
protected:
	ImageGenerator(const char blobType);

private:
	const char							mBlobType;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEGENERATOR_H_
