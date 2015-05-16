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

		// Sets the clippin' (visible) area of the scroll Area. 
		void				setScrollSize(const float newWidth, const float newHeight);

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

		const ci::Vec2f		getScrollerPosition();
		void				resetScrollerPosition();

	private:
		virtual void		updateServer(const ds::UpdateParams& p);
		void				scrollerUpdated(const ci::Vec2f scrollPos);
		void				scrollerTweenUpdated();
		void				checkBounds();
		void				handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);

		Sprite*				mScroller;
		bool				mScrollable;
		Momentum			mSpriteMomentum;
		float				mReturnAnimateTime;

		GradientSprite*		mTopFade;
		GradientSprite*		mBottomFade;
		float				mFadeHeight;
		ci::ColorA			mFadeTransColor;
		ci::ColorA			mFadeFullColor;
		bool				mTopFadeActive;
		bool				mBottomFadeActive;

		bool				mVertical;

		std::function<void(ScrollArea*)>	mScrollUpdatedFunction;

};

} // namespace ui
} // namespace ds
#endif