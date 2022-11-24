#pragma once
#ifndef ESSENTIALS_DS_UI_SPRITE_PNG_SEQUENCE_SPRITE_H_
#define ESSENTIALS_DS_UI_SPRITE_PNG_SEQUENCE_SPRITE_H_

#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds { namespace ui {

	/**
	 * \class PngSequenceSprite
	 */
	class PngSequenceSprite : public ds::ui::Sprite {
	  public:
		// what to do when you get to the end
		typedef enum { Loop = 0, Once } LoopStyle;

		// Create a png sequence using the image files speficied in the vector.
		// The string should be loadable by a normal ds::ui::Image
		// Default behavior: playing, one image per server frame, looping
		PngSequenceSprite(SpriteEngine& engine);
		PngSequenceSprite(SpriteEngine& engine, const std::vector<std::string>& imageFiles);

		void setImages(const std::vector<std::string>& imageFiles);

		// The number of seconds to wait for the next frame.
		// Default = 0.0, which will play each png frame on every server frame
		void setFrameTime(const float time);

		// Sets the looping method.
		// Loop = start over from the beginning
		// Once = play to the end and pause.
		void			setLoopStyle(LoopStyle style);
		const LoopStyle getLoopStyle() const;

		// Play = advance through the frames
		void play();
		// Pause = show the current frame
		void pause();

		// Are the frames advancing?
		const bool isPlaying() const;

		/// \brief returns true if all images are loaded.
		virtual bool isLoaded() const;

		// Jump to a specific frame.
		// Frame indices outside the range of the frames are ignored.
		void	  setCurrentFrameIndex(const int frameIndex);
		const int getCurrentFrameIndex() const;
		const int getNumberOfFrames();

		// Returns the image sprite at the index supplied.
		// If the index is invalid, returns nullptr
		// Don't release this frame, it'll cause trubs
		ds::ui::Image* getFrameAtIndex(const int frameIndex);

		// Makes the size of this sprite the same size as the first image
		// If there are no images, the size will be 0,0
		void sizeToFirstImage();

		// Sets the callback that runs when an animation is done.
		void setAnimationEndedCallback(const std::function<void()>& func);

		// Sets the callback that is called when the images have all loaded.
		void setLoadedCallback(const std::function<void()>& func);

		/// Change image flags used for internal Image sprites.
		/// Default is: ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F
		void setImageFlags(const int flags) { mImageFlags = flags; }

	  private:
		void		 checkLoaded();
		virtual void onUpdateServer(const ds::UpdateParams& p) override;
		void		 runAnimationEndedCallback();

		int							mImageFlags = ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F;
		LoopStyle					mLoopStyle;
		int							mCurrentFrameIndex;
		int							mNumFrames;
		bool						mPlaying;
		bool						mIsLoaded;
		float						mFrameTime;
		double						mLastFrameTime;
		std::vector<ds::ui::Image*> mFrames;
		ds::ui::Image*				mVisibleFrame = nullptr;

		std::function<void()> mAnimationEndedCallback;
		std::function<void()> mLoadedCallback;
	};

}} // namespace ds::ui

#endif
