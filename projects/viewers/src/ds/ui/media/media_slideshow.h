#pragma once

#include "media_viewer_settings.h"
#include <ds/data/resource.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {
	class MediaViewer;
	class MediaInterface;

	/**
	 * \class MediaSlideshow
	 *			Holds a series of MediaViewers that can be viewed sequentially
	 */
	class MediaSlideshow : public ds::ui::Sprite {
	  public:
		MediaSlideshow(ds::ui::SpriteEngine& eng);

		void clearSlideshow();
		void setMediaSlideshow(const std::vector<ds::Resource>& resources);

		void gotoNext(const bool wrap = true);
		void gotoPrev(const bool wrap = true);
		void gotoItemIndex(const int newIndex);

		void stopAllContent();

		void layout();

		void setAnimateDuration(const float animateDuration) { mAnimateDuration = animateDuration; }

		void setItemChangedCallback(std::function<void(const int currentItemIndex, const int totalItems)>);

		virtual void userInputReceived();

		/// Applies the same media viewer settings to every media viewer in the slideshow. Set before setting the
		/// slideshow content
		void setMediaViewerSettings(const MediaViewerSettings& settings);

		/// When loading a slide, will also load the next and previous slides for speed.
		/// If this is false, will only load the current slide
		void allowLoadAhead(const bool loadAhead);

		/// if true, will have empty areas on top or sides to show the whole media item, false will fill the area and
		/// crop (if clipping is on) default is true
		void setLetterboxed(const bool letterbox);

		/// Get the list of viewers. Be gentle with these
		/// The slideshow has ownership of these, so releasing these may have unintended consequences.
		/// These are also not guaranteed to be initialized
		std::vector<MediaViewer*>& getViewers() { return mViewers; }

	  protected:
		std::vector<MediaViewer*> mViewers;
		ds::ui::Sprite*			  mHolder;
		int						  mCurItemIndex;
		float					  mAnimateDuration;
		MediaViewerSettings		  mMediaViewerSettings;
		bool					  mAllowLoadAhead;
		bool					  mLetterBoxed;

		MediaInterface*														  mCurrentInterface;
		std::function<void(const int currentItemIndex, const int totalItems)> mItemChangedCallback;

		void recenterSlides();

		virtual void onSizeChanged();

		void loadCurrentAndAdjacent();
		void setCurrentInterface();
	};

}} // namespace ds::ui
