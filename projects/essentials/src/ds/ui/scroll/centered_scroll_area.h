#pragma once
#ifndef DS_UI_SCROLL_CENTERED_SCROLL_AREA
#define DS_UI_SCROLL_CENTERED_SCROLL_AREA

#include <ds/ui/scroll/scroll_area.h>

namespace ds {
namespace ui {

// This class is useful for snapping to the "center" of a set of evenly-spaced items, usually thumbnails.
// It works if the sole sprite added to the scroller is a LayoutSprite.

class CenteredScrollArea : public ScrollArea {
	public:
		CenteredScrollArea(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool vertical = true);

		// center by is how many items appear centered when snapping
		void			setCenterBy(int centerBy);
		int				getCenterBy() { return mCenterBy; }

		// getItemCount returns the number of children of the first child of the scroller.
		int				getItemCount();

		// getFirstChild returns the first sprite added to the scroller.
		// That sprite should be a LayoutSprite.
		ds::ui::Sprite*	getFirstChild();

		// This gets the item currently in the center, or just left of center for an even-valued center by parameter.
		int				getCenterIndex();

		// centerOnIndex moves the scroller to center on that item no matter what.
		// It does not respect the center by parameter.
		// It can be useful for temporarily moving an item to the center in a tween, and then following up with a postFunc.
		void			centerOnIndex(int index, float duration = 0.0f, float delay = 0.0f, const ci::EaseFn& ease = ci::easeNone, const std::function<void()>& postFunc = nullptr);
		
		// balanceOnIndex moves the scroller to center on an item while respecting the center by parameter.
		// If you want to respond to the end of the tween, pass in a postFunc.
		void			balanceOnIndex(int index, float duration = 0.0f, float delay = 0.0f, const ci::EaseFn& ease = ci::easeNone, const std::function<void()>& postFunc = nullptr);
		
	protected:
		// This override only activates if the object has not had a snap callback set directly.
		virtual bool	callSnapToPositionCallback(bool& doTween, ci::Vec3f& tweenDestination);
		
		bool			isOdd() { return ((mCenterBy % 2) == 1); }
		float			trueCenterOfItem(int index);
		float			trueCenterAfterItem(int index);
		float			relaxedCenterOfItem(int& index);
		
		int				mCenterBy;
		int				mCenterIndex;
};

} // namespace ui
} // namespace ds
#endif