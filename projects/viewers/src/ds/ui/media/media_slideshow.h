#pragma once
#ifndef DS_UI_MEDIA_MEDIA_SLIDESHOW
#define DS_UI_MEDIA_MEDIA_SLIDESHOW


#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>

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

protected:
	std::vector<MediaViewer*>			mViewers;
	ds::ui::Sprite*						mHolder;
	int									mCurItemIndex;
	float								mAnimateDuration;

	MediaInterface*						mCurrentInterface;

	void								recenterSlides();

	virtual void						onSizeChanged();

	void								loadCurrentAndAdjacent();
	void								setCurrentInterface();
};

} // namespace ui
} // namespace ds

#endif
