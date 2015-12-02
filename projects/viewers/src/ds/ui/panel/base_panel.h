#pragma once
#ifndef UI_PANEL_PANEL_BASE
#define UI_PANEL_PANEL_BASE

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/touch/momentum.h>

namespace ds {
namespace ui {


/** A base class to build a moveable, resizeable, bounds-checked, size checked panel.
The size of the panel will include the borders (top/left/right/bottom pad), but the content size is the resizeable part.
Generally, you'll have a piece of media as the content, with some interface buttons on the padding areas.
When the panel resizes, the content will get larger, but the interface will stay the same size.
This class provides no UI.

Usage:
- Add your content as a child
- Optionally set the padding of the viewer and the default size variable
- Set the ContentAspectRatio of the content (getWidth()/getHeight)
- Set the size of this sprite using setSize()
- Call setSizeLimits();
- Call setViewerSize(mDefaultSize.x, mDefaultSize.y);
- Your viewer is now setup at a default size.

Touch
- If you want, you can still use setTapFn() setSwipeCallback(), etc
- This sprite can have it's scale tweened, but as soon as it is touched, it will snap the scale to 1.0
- This is designed to resize, not rescale, when pinch-zoomed
- Change mBoundingArea if needed

Don't's
- Don't change the Center of the sprite
- Don't change the size of this sprite in the layout function

*/
class BasePanel : public ds::ui::Sprite {
public:
	BasePanel(ds::ui::SpriteEngine& engine);


	/** The content is the resizeable part of the panel.
	The L/R/T/B padding is added to the content width and height to produce the final size of the panel */
	void							setViewerSize(const float contentWidth, const float contentHeight);
	void							setViewerSize(const ci::Vec2f newContentSize);

	/** Calls the above function, but using the content aspect ratio so it's proportional, and the padding is added to the outside */
	void							setViewerWidth(const float contentWidth);

	/** If you animate this thing, call this before the tween starts (And be sure to call tweenEnded when it completes!) */
	void							tweenStarted();
	/** Re-enables many functions after a tween completes */
	void							tweenEnded();

	/** Change the region used in checkBounds(), relative to this sprite's parent. */
	void							setBoundingArea(const ci::Rectf& newBoundingRegion){ mBoundingArea = newBoundingRegion; }

	/** Automatically handles enable/disable and adds padding */
	void							animateToDefaultSize();

	/** Automatically handles enable/disable and adds padding, so the contentSize should be the x/y of the destination of your content */
	void							animateSizeTo(const ci::Vec2f newContentSize);
	void							animateWidthTo(const float newWidth);
	void							aniamteHeightTo(const float newHeight);

	const ci::Vec2f&				getMinSize(){ return mMinSize; }
	const ci::Vec2f&				getDefaultSize(){ return mDefaultSize; }

	/** Sets the default size. Careful here, the aspect ratio of this should match the content aspect ratio. */
	void							setDefaultSize(const ci::Vec2f& defaultSize){ mDefaultSize = defaultSize; }

	void							setAnimateDuration(const float animDuration){ mAnimDuration = animDuration; }

	/** Called when the panel lays itself out, so you can add sprites to this panel without extending the panel class.
	This is called after the base layout function.*/
	void							setLayoutCallback(std::function<void()> layoutCallback);

	/** Force a refresh of size and position. Do not override, use onLayout() for overrides */
	virtual  void					layout() final;

	const float						getContentAspectRatio(){ return mContentAspectRatio; }

protected:
	virtual void					updateServer(const ds::UpdateParams &updateParams);


	/** Override this to layout your ui when the panel changes size .
	Don't change the size of this sprite in this function (you'll get an infinite loop) */
	virtual void					onLayout(){};

	void							handleTouchInfo(const ds::ui::TouchInfo& ti);
	void							setSizeLimits();
	void							checkBounds(const bool immediate = false);

	float							mContentAspectRatio;
	float							mTopPad;
	float							mLeftPad;
	float							mRightPad;
	float							mBottomPad;

	ci::Vec2f						mMinSize;
	ci::Vec2f						mDefaultSize;
	ci::Vec2f						mMaxSize;

	ds::Momentum					mMomentum;
	bool							mTouching;

	float							mAnimDuration;
	bool							mAnimating;
	ci::Rectf						mBoundingArea;

	std::function<void()>			mLayoutCallback;

private:

	virtual void					onSizeChanged();
};

} // namespace ui
} // namespace ds

#endif // UI_PANEL_PANEL_BASE
