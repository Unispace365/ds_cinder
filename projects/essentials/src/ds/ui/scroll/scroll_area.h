#pragma once
#ifndef DS_UI_SCROLL_SCROLL_AREA
#define DS_UI_SCROLL_SCROLL_AREA

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/touch/momentum.h>
#include <functional>
#include <ds/ui/sprite/gradient_sprite.h>

namespace ds {
namespace ui {

class ScrollArea : public ds::ui::Sprite {
	public:

		ScrollArea(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool verticalScrolling = true);

		void				setVertical(bool vertical);

		// Sets the clippin' (visible) area of the scroll Area. 
		void				setScrollSize(const float newWidth, const float newHeight);

          //Temproarily enable/disable touches
		void				enableScrolling(bool enable);

		// Set the position of your sprite before you add it here.
		// The scroll area will be sized after the new sprite is passed in.
		void				addSpriteToScroll(ds::ui::Sprite* bs);

		// if you have buttons or whatnot in the scroll area, when you start dragging
		// you'll want to pass the touch to a sprite, the sprite returned by this function.
		Sprite*				getSpriteToPassTo();

		const bool			getScrollable() const { return mScrollable; }

		// turns on fades on the top and bottom (vertical scrolling) or left and right (horizontal) if there is more content in that direction
		void				setFadeColors(ci::ColorA fadeColorFull, ci::ColorA fadeColorTransparent);
		void				setUseFades(const bool doFading);
		void				setFadeHeight(const float fadeHeight);

		void				setScrollUpdatedCallback(const std::function<void(ScrollArea* thisThing)> &func);
		void				setTweenCompleteCallback(const std::function<void(ScrollArea*)>& func);
		void				setSnapToPositionCallback(const std::function<void(ScrollArea*, Sprite*, bool&, ci::vec3&)>& func);
		void				setScrollerTouchedCallback(const std::function<void()>& func); // just lets you know some interaction happened with the scroller

		const ci::vec2		getScrollerPosition();
		void				resetScrollerPosition();

		// For external UI use. The 0.0 - 1.0 percent of the scroll. 0.0 == the start (top in vertical scrolls). 1.0 == the bottom (fully scrolled through the list)
		float				getScrollPercent();
		void				setScrollPercent(const float percenty);

		// How much of the scroller is currently visible. If the scroller is smaller than the scroll area, then this will be 1.0
		float				getVisiblePercent();

		// Move the scroll forwards or backwards by a "page", defined by the visible area minus the size of the fades (if present)
		// May not work correctly in perspective
		void				scrollPage(const bool forwards, const bool animate = true);

		// If this scroll area is rotated globally, rotate the touch delta by that amount. Default = false
		void				handleTouchesRotated(const bool doRotated){ mHandleRotatedTouches = doRotated; }

	protected:
		virtual void		updateServer(const ds::UpdateParams& p);
		virtual void		onSizeChanged();
		void				scrollerUpdated(const ci::vec2 scrollPos);
		void				scrollerTweenUpdated();
		void				tweenComplete();
		void				checkBounds();
		void				handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);
		virtual bool		callSnapToPositionCallback(bool& doTween, ci::vec3& tweenDestination);

		Sprite*				mScroller;
		bool				mScrollable;
		Momentum			mSpriteMomentum;
		float				mReturnAnimateTime;
		bool				mWillSnapAfterDelay;

		GradientSprite*		mTopFade;
		GradientSprite*		mBottomFade;
		float				mFadeHeight;
		ci::ColorA			mFadeTransColor;
		ci::ColorA			mFadeFullColor;
		bool				mTopFadeActive;
		bool				mBottomFadeActive;
		bool				mHandleRotatedTouches;

		bool				mVertical;

		// the current 0.0 - 1.0 percent of the scroll position
		// Only used for external ui, not internally
		float				mScrollPercent;

		std::function<void(ScrollArea*)>	mScrollUpdatedFunction;
		std::function<void(ScrollArea*)>	mTweenCompleteFunction;
		std::function<void(ScrollArea*, Sprite*, bool&, ci::vec3&)>	mSnapToPositionFunction;
		std::function<void()>				mScrollerTouchedFunction;

};

} // namespace ui
} // namespace ds
#endif