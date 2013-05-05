#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGESOURCE_H_
#define DS_UI_IMAGESOURCE_IMAGESOURCE_H_

#include <cinder/gl/Texture.h>

namespace ds {
class DataBuffer;

namespace ui {

/**
 * \class ds::ui::ImageSource
 * \brief A Source is responsible for creating the actual image.
 */
class ImageSource {
public:
	virtual ~ImageSource();

	virtual const ci::gl::Texture*		getImage() = 0;

	char															getBlobType() const;
	virtual void											writeTo(DataBuffer&) const = 0;
	virtual bool											readFrom(DataBuffer&) = 0;

protected:
	ImageSource(const char blobType);

private:
	const char												mBlobType;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGESOURCE_H_
