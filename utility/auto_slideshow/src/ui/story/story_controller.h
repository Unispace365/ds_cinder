#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

class StoryView;

/**
* \class downstream::StoryController
*			Controls and manages any stories onscreen. If you don't rename this class into something relevant I will publicly shame you.
*/
class StoryController : public ds::ui::SmartLayout {
public:
	StoryController(ds::ui::SpriteEngine& eng);


	void rotatePhoto();
	void removePhoto(StoryView* sv);

	StoryView*	mStoryA = nullptr;
	StoryView*	mStoryB = nullptr;
	bool mOnA = false;
	int mCurrentPhotoIndex = 0;

	std::vector<ds::model::ContentModelRef> mPhotoz;
};

} // namespace downstream


