#pragma once
#ifndef DS_IMAGEWITHTHUMBNAIL_H
#define DS_IMAGEWITHTHUMBNAIL_H

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/image.h>
#include "ds/data/resource.h"

namespace ds {
namespace ui {

/*!
 * @class     ImageWithThumbnail
 * @namespace ds::ui
 * @brief     A Sprite that draws an image on-screen
 */
class ImageWithThumbnail : public Image {
public:
	static const float DEFAULT_FADE_DURATION;

	/// @cond Constructor overloads
	
	/// @brief constructs an image sprite and shows the thumbnail until the main image is ready
	ImageWithThumbnail(SpriteEngine& engine, const int flags = 0, float fadeDuration = DEFAULT_FADE_DURATION);
	ImageWithThumbnail(SpriteEngine& engine, const std::string& filename, const int flags = 0, float fadeDuration = DEFAULT_FADE_DURATION);
	ImageWithThumbnail(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags = 0, float fadeDuration = DEFAULT_FADE_DURATION);
	ImageWithThumbnail(SpriteEngine& engine, const ds::Resource& resource, const int flags = 0, float fadeDuration = DEFAULT_FADE_DURATION);

	/// @endcond

public:
	float						getFadeDuration() { return mFadeDuration; }
	void						setFadeDuration(float fadeDuration) { mFadeDuration = fadeDuration; };

	// this is our interception point for duplicating the resource with a thumbnail (if any)
	virtual void				setImageResource(const ds::Resource& resource, const int flags = 0);

	/// @brief returns true if the last requested image is loaded as a texture
	virtual bool				isLoaded() const;
	bool						isLoadedThumbnail(bool thumbnail) const;

	virtual void				setCircleCrop(bool circleCrop);
	virtual void				setCircleCropRect(const ci::Rectf& rect);

protected:
	virtual bool				isLoadedPrimary() const;
	
	/// @cond image status callbacks
	virtual void				onImageLoaded();
	/// @endcond

	virtual void				onSizeChanged();

private:
	typedef Image				inherited;

	Image*						mThumbnail;
	float						mFadeDuration;

};

} // namespace ui
} // namespace ds

#endif //DS_IMAGEWITHTHUMBNAIL_H
