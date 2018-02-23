#pragma once
#ifndef _VIDEO_LEAK_TESTER_APP_UI_STORY_STORY_VIEW_H_
#define _VIDEO_LEAK_TESTER_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

namespace downstream {

class Globals;

/**
* \class downstream::StoryView
*			A sample view
*/
class StoryView final : public ds::ui::Sprite {
public:
	StoryView(Globals& g);

private:

	void								restartSprite();
	void								restartText();

	Globals&							mGlobals;

	ds::ui::Sprite*						mASprite;

	ds::ui::Text*						mTheText;

};

} // namespace downstream

#endif

