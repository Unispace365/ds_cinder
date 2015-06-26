#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEOWNER_H_
#define DS_UI_IMAGESOURCE_IMAGEOWNER_H_

#include "ds/data/resource.h"
#include "ds/ui/image_source/image_client.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageOwner
 * \brief A convenience for anyone that wants to own an image source.
 * I store the image source object and provide a collection of
 * convenient setter-functions.
 */
class ImageOwner {
public:
	virtual ~ImageOwner();

	// Basic image source set API
	void				setImage(const ImageSource&);
	void				clearImage();

	// Type conveniences
	/** Loads an image based on the filename.
	 * \param filename is the absolute file path to the resource.
	 * \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	 */
	void				setImageFile(const std::string& filename, const int flags = 0);

	/** Loads an image based on the resource.
	 * \param resource is the resource.
	 * \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	 */
	void				setImageResource(const ds::Resource& resource, const int flags = 0);

	/** Loads an image based on the resource id.
	* \param resourceId is the resource.
	* \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	*/
	void				setImageResource(const ds::Resource::Id& resourceId, const int flags = 0);

protected:
	virtual void		onImageChanged();
	ImageOwner(ds::ui::SpriteEngine&);
	ImageClient			mImageSource;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEOWNER_H_
