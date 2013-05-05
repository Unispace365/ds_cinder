#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGECLIENT_H_
#define DS_UI_IMAGESOURCE_IMAGECLIENT_H_

#include <functional>
#include <cinder/gl/Texture.h>

namespace ds {
class DataBuffer;

namespace ui {
class ImageGenerator;
class ImageSource;
class SpriteEngine;

/**
 * \class ds::ui::ImageClient
 * \brief An image client is an abstracted way for clients to
 * access images. An ImageClient is responsible for returning an
 * image, nothing else. It makes use of an ImageSource as the public
 * API that provides config info, and an ImageGenerator to generate
 * the actual image.
 */
class ImageClient
{
public:
	ImageClient(ds::ui::SpriteEngine&);
	~ImageClient();

	void												clear();
	// Set the device responsible for generating the image
	void												setSource(const ImageSource&);

	// Answer the generator image. If the texture is not null, then it will be valid.
	const ci::gl::Texture*			getImage();

	void												writeTo(DataBuffer&) const;
	bool												readFrom(DataBuffer&);

private:
	ds::ui::SpriteEngine&				mEngine;
	ImageGenerator*							mGenerator;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGECLIENT_H_
