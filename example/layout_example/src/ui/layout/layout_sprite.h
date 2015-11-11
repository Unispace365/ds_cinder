#pragma once
#ifndef DS_UI_LAYOUT_LAYOUT_SPRITE
#define DS_UI_LAYOUT_LAYOUT_SPRITE


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace example {

/**
* \class example::StoryView
*			A sample view
*/
class LayoutSprite : public ds::ui::Sprite  {
public:
	LayoutSprite(ds::ui::SpriteEngine& engine);

	typedef enum { kLayoutNone, kLayoutVFlow, kLayoutHFlow } LayoutType;
	enum { kFixedSize = 0, kFlexSize, kStretchSize } SizeType;

	void					runLayout();

	const LayoutType&		getLayoutType(){ return mLayoutType; }
	void					setLayoutType(const LayoutType& typey){ mLayoutType = typey; }

	void					setLayoutUpdatedFunction(const std::function<void()> layoutUpdatedFunction);
	void					onLayoutUpdate();

	float					getSpacing(){ return mSpacing; }
	void					setSpacing(const float spacing){ mSpacing = spacing; }

private:
	std::function<void()>	mLayoutUpdatedFunction;

	float					mSpacing;
	LayoutType				mLayoutType;

};

} // namespace example

#endif
