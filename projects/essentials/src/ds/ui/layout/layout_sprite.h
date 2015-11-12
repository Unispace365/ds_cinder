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

	// None = No action taken on elements, but will run recursive layouts
	// VFlow = Size elements based on their size settings, then position them, top-to-bottom
	// HFlow = Same as VFlow, but horizontally from left-to-right
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

	/// Returns the spacing between each element in the layout (use padding on each element to do add specific spacing)
	float					getSpacing(){ return mSpacing; }
	/// Sets the spacing between each element in the layout (use padding on each element to do add specific spacing)
	void					setSpacing(const float spacing){ mSpacing = spacing; }



protected:
	// See enum declaration for descriptions
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runVLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runHLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runNoneLayout();
	std::function<void()>	mLayoutUpdatedFunction;

	float					mSpacing;
	LayoutType				mLayoutType;

};

} // namespace ui
} // namespace ds

#endif
