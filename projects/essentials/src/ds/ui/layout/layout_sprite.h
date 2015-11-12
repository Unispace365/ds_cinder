#pragma once
#ifndef DS_UI_LAYOUT_LAYOUT_SPRITE
#define DS_UI_LAYOUT_LAYOUT_SPRITE


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::LayoutSprite
*		A sprite that can run recursive flow layouts. Children can be normal sprites or other layouts.
*/
class LayoutSprite : public ds::ui::Sprite  {
public:
	LayoutSprite(ds::ui::SpriteEngine& engine);

	typedef enum { kLayoutNone, kLayoutVFlow, kLayoutHFlow } LayoutType;
	enum { kFixedSize = 0, kFlexSize, kStretchSize } SizeType;
	enum { kLeft = 0, kCenter, kRight } HAlignment;
	enum { kTop = 0, kMiddle, kBottom } VAlignment;

	// Fits the sprite supplied into the target area
	static void				fitInside(ds::ui::Sprite* sp, const ci::Rectf area, const bool letterbox);

	void					runLayout();

	const LayoutType&		getLayoutType(){ return mLayoutType; }
	void					setLayoutType(const LayoutType& typey){ mLayoutType = typey; }

	void					setLayoutUpdatedFunction(const std::function<void()> layoutUpdatedFunction);
	void					onLayoutUpdate();

	float					getSpacing(){ return mSpacing; }
	void					setSpacing(const float spacing){ mSpacing = spacing; }

protected:
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runVLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runHLayout();

	std::function<void()>	mLayoutUpdatedFunction;

	float					mSpacing;
	LayoutType				mLayoutType;

};

} // namespace ui
} // namespace ds

#endif
