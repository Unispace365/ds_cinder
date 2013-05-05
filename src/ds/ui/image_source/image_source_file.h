#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGESOURCEFILE_H_
#define DS_UI_IMAGESOURCE_IMAGESOURCEFILE_H_

#include "ds/ui/image_source/image_source.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {
class ImageSourceRegistry;

namespace ui {
class SpriteEngine;

/**
 * \class ds::ui::ImageSourceFile
 * \brief Load an image from a file.
 */
class ImageSourceFile : public ImageSource
{
public:
  /**
   * \param filename is the filename (and path) for the resource.
   * \param flags provides scope info (i.e. ds::IMG_CACHE).
   */
	ImageSourceFile(SpriteEngine&, const std::string& filename, const int flags = 0);

	const ci::gl::Texture*		getImage();

	virtual void							writeTo(DataBuffer&) const;
	virtual bool							readFrom(DataBuffer&);

private:
	ImageSourceFile(SpriteEngine&);

	ImageToken								mImageToken;
	std::string								mFilename;
	int												mFlags;
	ci::gl::Texture						mTexture;

	// Engine initialization
public:
	// Generators must be registered with the system at startup (i.e. in App constructor)
	static void								install(ds::ImageSourceRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGESOURCE_H_
