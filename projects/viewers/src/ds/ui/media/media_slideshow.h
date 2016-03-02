#pragma once
#ifndef DS_UI_MEDIA_MEDIA_SLIDESHOW
#define DS_UI_MEDIA_MEDIA_SLIDESHOW


#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>
#include "media_viewer_settings.h"

namespace ds {
namespace ui {
class MediaViewer;
class MediaInterface;

/**
* \class ds::ui::MediaSlideshow
*			Holds a series of MediaViewers that can be viewed sequentially
*/
class MediaSlideshow : public ds::ui::Sprite  {
public:
	MediaSlideshow(ds::ui::SpriteEngine& eng);

	void								clearSlideshow();
	void								setMediaSlideshow(const std::vector<ds::Resource>& resources);

	void								gotoNext(const bool wrap = true);
	void								gotoPrev(const bool wrap = true);
	void								gotoItemIndex(const int newIndex);

	void								stopAllContent();

	void								layout();

	void								setAnimateDuration(const float animateDuration){ mAnimateDuration = animateDuration; }

	void								setItemChangedCallback(std::function<void(const int currentItemIndex, const int totalItems)>);

	virtual void						userInputReceived();

	/// Applies the same media viewer settings to every media viewer in the slideshow. Set before setting the slideshow content
	void								setMediaViewerSettings(const MediaViewerSettings& settings);

protected:
	std::vector<MediaViewer*>			mViewers;
	ds::ui::Sprite*						mHolder;
	int									mCurItemIndex;
	float								mAnimateDuration;
	MediaViewerSettings					mMediaViewerSettings;

	MediaInterface*						mCurrentInterface;
	std::function<void(const int currentItemIndex, const int totalItems)> mItemChangedCallback;

	void								recenterSlides();

	virtual void						onSizeChanged();

	void								loadCurrentAndAdjacent();
	void								setCurrentInterface();
};

} // namespace ui
} // namespace ds

#endif
