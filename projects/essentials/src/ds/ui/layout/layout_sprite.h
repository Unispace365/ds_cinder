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
	// Size = only adjust the size of the children, but do not position
	typedef enum { kLayoutNone, kLayoutVFlow, kLayoutHFlow, kLayoutSize } LayoutType;

	// FixedSize = Sprite sized to mLayoutSize (on Sprite.h), or left alone if it's not set
	// FlexSize = Sprite is adjusted to fit the layout. For example, in a V layout, the text is resized to the width, and the height is calculated
	// StretchSize = Sprite will fill to the rest of the available space. Multiple Stretch sprites will evenly split the remainder
	// FillSize = Sprite isn't used in V/H layout calculations, but is sized to the whole layout (minus padding). Great for backgrounds of flex size layouts.
	enum { kFixedSize = 0, kFlexSize, kStretchSize, kFillSize } SizeType;

	// In VFlow layouts, adjusts the x-position during layout
	enum { kLeft = 0, kCenter, kRight } HAlignment;
	// In HFlow layouts, adjusts the y-position during layout
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

	/// For V or H flow layouts, sets the overall alignment for the children. 
	/// Generally only has an effect for sizeType = kFixedSize, kStretchSize or fill; layoutType = kLayoutVFlow or kLayoutHFlow; and there are no stretch children
	void					setOverallAlignment(const int align){ mOverallAlign = align; }

protected:
	// See enum declaration for descriptions
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runVLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runHLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runNoneLayout();
	// virtual in case you want to override with your own layout jimmies.
	virtual void			runSizeLayout();

	void					runVOrHLayout(bool vertical);

	std::function<void()>	mLayoutUpdatedFunction;

	float					mSpacing;
	LayoutType				mLayoutType;
	int						mOverallAlign; // can align children if this is not a flex size and there are no stretch children

};

} // namespace ui
} // namespace ds

#endif
