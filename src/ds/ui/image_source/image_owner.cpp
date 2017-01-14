#include "stdafx.h"

#include "image_owner.h"

#include "image_file.h"
#include "image_resource.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageOwner
 */
ImageOwner::ImageOwner(ds::ui::SpriteEngine& e)
	: mImageSource(e) {
}

ImageOwner::~ImageOwner() {
}


void ImageOwner::setImage(const ImageSource& src) {
	mImageSource.setSource(src);
	onImageChanged();
}

void ImageOwner::clearImage() {
	mImageSource.clear();
	onImageChanged();
}

void ImageOwner::setImageFile(const std::string& filename, const int flags) {
	setImage(ImageFile(filename, flags));
}

void ImageOwner::setImageResource(const ds::Resource& r, const int flags) {
	setImage(ImageResource(r, flags));
}

void ImageOwner::setImageResource(const ds::Resource::Id& rid, const int flags) {
	setImage(ImageResource(rid, flags));
}


const ci::gl::TextureRef ImageOwner::getImageTexture(){
	return mImageSource.getImage();
}


std::string ImageOwner::getImageFilename(){
	return mImageSource.getImageFilename();
}

void ImageOwner::onImageChanged() {
}

} // namespace ui
} // namespace ds
