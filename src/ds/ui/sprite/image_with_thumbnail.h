#pragma once
#ifndef DS_IMAGEWITHTHUMBNAIL_H
#define DS_IMAGEWITHTHUMBNAIL_H

#include "ds/data/resource.h"
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {

	/*!
	 * @class     ImageWithThumbnail
	 * @brief     A Sprite that draws an image on-screen
	 */
	class ImageWithThumbnail : public Image {
	  public:
		static const float DEFAULT_FADE_DURATION;

		/// \brief constructs an image sprite and shows the thumbnail until the main image is ready
		ImageWithThumbnail(SpriteEngine& engine, const int flags = 0, float fadeDuration = DEFAULT_FADE_DURATION);
		/// \brief constructs an image sprite and shows the thumbnail until the main image is ready
		ImageWithThumbnail(SpriteEngine& engine, const std::string& filename, const int flags = 0,
						   float fadeDuration = DEFAULT_FADE_DURATION);
		/// \brief constructs an image sprite and shows the thumbnail until the main image is ready
		ImageWithThumbnail(SpriteEngine& engine, const ds::Resource::Id& resourceId, const int flags = 0,
						   float fadeDuration = DEFAULT_FADE_DURATION);
		/// \brief constructs an image sprite and shows the thumbnail until the main image is ready
		ImageWithThumbnail(SpriteEngine& engine, const ds::Resource& resource, const int flags = 0,
						   float fadeDuration = DEFAULT_FADE_DURATION);

		float getFadeDuration() const { return mFadeDuration; }
		void  setFadeDuration(float fadeDuration) { mFadeDuration = fadeDuration; };

		/// this is our interception point for duplicating the resource with a thumbnail (if any)
		virtual void setImageResource(const ds::Resource& resource, const int flags = 0);

		/// \brief returns true if the last requested image is loaded as a texture
		virtual bool isLoaded() const;
		/// \brief returns true if the last requested image's thumbnail is loaded as a texture
		bool isLoadedThumbnail(bool thumbnail) const;

		virtual void setCircleCrop(bool circleCrop);
		virtual void setCircleCropRect(const ci::Rectf& rect);

	  protected:
		virtual bool isLoadedPrimary() const;

		virtual void onImageLoaded();
		virtual void onSizeChanged();

	  private:
		typedef Image inherited;

		Image* mThumbnail;
		float  mFadeDuration;
	};

}} // namespace ds::ui

#endif // DS_IMAGEWITHTHUMBNAIL_H
