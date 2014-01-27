#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGERESOURCE_H_
#define DS_UI_IMAGESOURCE_IMAGERESOURCE_H_

#include <string>
#include "ds/data/resource.h"
#include "ds/ui/image_source/image_source.h"

namespace ds {
class ImageRegistry;

namespace ui {

/**
 * \class ds::ui::ImageResource
 * \brief Load an image from a resource.
 */
class ImageResource : public ImageSource {
public:
  /**
   * \param resource is the resource.
   * \param flags provides scope info (i.e. ds::IMG_CACHE).
   */
	ImageResource(const ds::Resource&, const int flags = 0);

	virtual ImageGenerator*		newGenerator(SpriteEngine&) const;

private:
	const ds::Resource			mResource;
	const int					mFlags;

	// Engine initialization
public:
	// Generators must be registered with the system at startup (i.e. in App constructor)
	static void					install(ds::ImageRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEFILE_H_
