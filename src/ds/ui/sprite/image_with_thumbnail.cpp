#include "stdafx.h"

#include "image_with_thumbnail.h"

#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

const float ImageWithThumbnail::DEFAULT_FADE_DURATION = 0.35f;

ImageWithThumbnail::ImageWithThumbnail(SpriteEngine& engine, const int flags, float fadeDuration)
	: inherited(engine, flags)
	, mThumbnail(nullptr)
	, mFadeDuration(fadeDuration)
{
	mLayoutFixedAspect = true;
}

ImageWithThumbnail::ImageWithThumbnail(SpriteEngine& engine, const std::string& filename, const int flags, float fadeDuration)
	: inherited(engine, filename, flags)
	, mThumbnail(nullptr)
	, mFadeDuration(fadeDuration)
{
	mLayoutFixedAspect = true;
}

ImageWithThumbnail::ImageWithThumbnail(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags, float fadeDuration)
	: inherited(engine, resourceId, flags)
	, mThumbnail(nullptr)
	, mFadeDuration(fadeDuration)
{
	mLayoutFixedAspect = true;
}

ImageWithThumbnail::ImageWithThumbnail(SpriteEngine& engine, const ds::Resource& resource, const int flags, float fadeDuration)
	: inherited(engine, flags)
	, mThumbnail(nullptr)
	, mFadeDuration(fadeDuration)
{
	setImageResource(resource, flags);
	mLayoutFixedAspect = true;
}

void ImageWithThumbnail::setImageResource(const ds::Resource& resource, const int flags){
	inherited::setImageResource(resource, flags);

	// dump an existing thumbnail, if any
	if(mThumbnail){
		mThumbnail->release();
		mThumbnail = nullptr;
	}

	checkStatus();

	if(!isLoadedPrimary()){
		if(resource.getThumbnailId() > 0 || !resource.getThumbnailFilePath().empty()){
			mThumbnail = new ds::ui::Image(mEngine);
			addChildPtr(mThumbnail);
			if(resource.getThumbnailId() > 0){
				mThumbnail->setImageResource(resource.getThumbnailId());
			} else {
				mThumbnail->setImageFile(resource.getThumbnailFilePath());
			}
			mThumbnail->setSize(getWidth(), getHeight());
		}
	}
}

bool ImageWithThumbnail::isLoaded() const{
	return isLoadedThumbnail(true);
}

bool ImageWithThumbnail::isLoadedThumbnail(bool thumbnail) const{
	bool output = true;
	if(mThumbnail && thumbnail){
		output = mThumbnail->isLoaded();
	} else {
		output = inherited::isLoaded();
	}
	return output;
}

bool ImageWithThumbnail::isLoadedPrimary() const{
	return isLoadedThumbnail(false);
}

void ImageWithThumbnail::setCircleCrop(bool circleCrop){
	inherited::setCircleCrop(circleCrop);
	if(mThumbnail){
		mThumbnail->setCircleCrop(circleCrop);
	}
}

void ImageWithThumbnail::setCircleCropRect(const ci::Rectf& rect){
	inherited::setCircleCropRect(rect);
	if(mThumbnail){
		// scale the rect before assigning to the thumbnail
		float thumbScale = mThumbnail->getScale().x;
		float inverseThumbScale = 1.0f;
		if(thumbScale > 0.01f){
			inverseThumbScale = 1.0f / thumbScale;
		}
		ci::Rectf scaleRect(
			rect.x1 * inverseThumbScale,
			rect.y1 * inverseThumbScale,
			rect.x2 * inverseThumbScale,
			rect.y2 * inverseThumbScale
		);
		mThumbnail->setCircleCropRect(scaleRect);
	}
}

void ImageWithThumbnail::onImageLoaded(){
	inherited::onImageLoaded();
	if(mThumbnail){
		// fade the thumbnail out and eliminate it
		mThumbnail->tweenOpacity(0.0f, mFadeDuration, 0.0f, ci::easeNone, [this](){
			mThumbnail->release();
			mThumbnail = nullptr;
		});
	}
}

void ImageWithThumbnail::onSizeChanged(){
	inherited::onSizeChanged();
	if(mThumbnail){
		mThumbnail->setSize(getWidth(), getHeight());
	}
}

} // namespace ui
} // namespace ds
