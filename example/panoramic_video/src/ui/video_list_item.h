#pragma once
#ifndef DS_UI_VIEWERS_COMPONENT_VIDEO_LIST_ITEM
#define DS_UI_VIEWERS_COMPONENT_VIDEO_LIST_ITEM

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/data/resource.h>

namespace panoramic{
class Globals;

class VideoListItem : public ds::ui::Sprite {
public:
	VideoListItem(Globals& g, const float widdy, const float hiddy);

	ds::Resource				getInfo();
	void						setInfo(const ds::Resource model);

	void						animateOn(const float delay);

	// Highlighted or not
	// 1 == highlighted, 0 = normal
	// TODO: make an enum or use a common one
	void						setState(const int buttonState);
	void						layout();

private:
	ds::Resource				mInfoModel;
	Globals&					mGlobals;

	ds::ui::LayoutSprite*		mLayout;
	ds::ui::Image*				mThumbnail;
	ds::ui::Sprite*				mBackground;
	ds::ui::Text*				mLabel;

};

}

#endif

