#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEFILE_H_
#define DS_UI_IMAGESOURCE_IMAGEFILE_H_

#include <string>
#include "ds/ui/image_source/image_source.h"

namespace ds {
class ImageRegistry;

namespace ui {

/**
 * \class ds::ui::ImageFile
 * \brief Load an image from a file.
 */
class ImageFile : public ImageSource
{
public:
  /**
   * \param filename is the filename (and path) for the resource.
   * \param flags provides scope info (i.e. ds::IMG_CACHE).
   */
	ImageFile(const std::string& filename, const int flags = 0);

	virtual ImageGenerator*		newGenerator(SpriteEngine&) const;
	virtual bool				generatorMatches(const ImageGenerator&) const;

private:
	const std::string			mFilename;
	const int					mFlags;

	// Engine initialization
public:
	// Generators must be registered with the system at startup (i.e. in App constructor)
	static void					install(ds::ImageRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEFILE_H_
