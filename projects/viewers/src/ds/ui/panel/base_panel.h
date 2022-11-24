#pragma once
#ifndef UI_PANEL_PANEL_BASE
#define UI_PANEL_PANEL_BASE

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/touch/momentum.h>

namespace ds { namespace ui {


	/** A base class to build a moveable, resizeable, bounds-checked, size checked panel.
	The size of the panel will include the borders (top/left/right/bottom pad), but the content size is the resizeable
	part. Generally, you'll have a piece of media as the content, with some interface buttons on the padding areas. When
	the panel resizes, the content will get larger, but the interface will stay the same size. This class provides no
	UI.

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
		void setViewerSize(const float contentWidth, const float contentHeight);
		void setViewerSize(const ci::vec2 newContentSize);

		/** Calls the above function, but using the content aspect ratio so it's proportional, and the padding is added
		 * to the outside */
		void setViewerWidth(const float contentWidth);
		void setViewerHeight(const float contentHeight);

		/** If you animate this thing, call this before the tween starts (And be sure to call tweenEnded when it
		 * completes!) */
		void tweenStarted();
		/** Re-enables many functions after a tween completes */
		void tweenEnded();

		/** Sets the flag that this viewer is on it's way out. Usage up to client app*/
		void setAboutToBeRemoved(const bool isRemoving = true);

		/** Gets the flag that this panel will be removed or retired after the current animation. This is primarily for
		 * client app logic. */
		bool getIsAboutToBeRemoved() { return mRemoving; }

		/** Change the region used in checkBounds(), relative to this sprite's parent. */
		void setBoundingArea(const ci::Rectf& newBoundingRegion) { mBoundingArea = newBoundingRegion; }

		/** Automatically handles enable/disable and adds padding */
		void animateToDefaultSize();

		/** Automatically handles enable/disable and adds padding, so the contentSize should be the x/y of the
		 * destination of your content */
		void animateSizeTo(const ci::vec2 newContentSize);
		void animateWidthTo(const float newWidth);
		void animateHeightTo(const float newHeight);

		const ci::vec2& getMinSize() { return mMinSize; }
		const ci::vec2& getDefaultSize() { return mDefaultSize; }

		/** Used in the setSizeLimits() function, so this must be set before calculating the size limits.
			NOTE: the actual size limits are NOT calculated when calling this function, that must be done by the
		   override class after this.*/
		void setAbsoluteSizeLimits(const ci::vec2& absMin, const ci::vec2& absMax);

		/** Calculate the size limits */
		void setSizeLimits();

		/** Sets the default size. Careful here, the aspect ratio of this should match the content aspect ratio. */
		void setDefaultSize(const ci::vec2& defaultSize) { mDefaultSize = defaultSize; }

		void		setAnimateDuration(const float animDuration) { mAnimDuration = animDuration; }
		const float getAnimateDuration() { return mAnimDuration; }

		/** Called when the panel lays itself out, so you can add sprites to this panel without extending the panel
		class. This is called after the base layout function.*/
		void setLayoutCallback(std::function<void()> layoutCallback);

		/** Force a refresh of size and position. Do not override, use onLayout() for overrides */
		virtual void layout() final;

		const float getContentAspectRatio() { return mContentAspectRatio; }

		void checkBounds(const bool immediate = false);

		/** Sends this panel to the front and calls onPanelActivated() */
		void activatePanel();

		/** If enabled (the default), will send this panel to the front on any user input. Otherwise leaves the order
		 * alone */
		void setAutoKeepInFront(const bool autoBringToFront) { mAutoSendToFront = autoBringToFront; }

		/** The panel was dragged or it's bounds checked. This is very loose, and might be called from the update loop
		 * and multiple times per actual position change */
		void setPositionUpdatedCallback(std::function<void()> posUpdateCallback);

	  protected:
		virtual void onUpdateServer(const ds::UpdateParams& updateParams);


		/** Override this to layout your ui when the panel changes size .
		Don't change the size of this sprite in this function (you'll get an infinite loop) */
		virtual void onLayout(){};

		/** The About to be removed flag has just been set */
		virtual void onAboutToBeRemoved(){};

		/** When this panel has been sent to the front via activatePanel() */
		virtual void onPanelActivated() {}

		virtual void userInputReceived() override;

		void handleTouchInfo(const ds::ui::TouchInfo& ti);

		float mContentAspectRatio;
		float mTopPad;
		float mLeftPad;
		float mRightPad;
		float mBottomPad;

		// Abs min is used when calculating size limits
		ci::vec2 mAbsMinSize;
		// Abs max is used when calculating size limits
		ci::vec2 mAbsMaxSize;
		ci::vec2 mMinSize;
		ci::vec2 mDefaultSize;
		ci::vec2 mMaxSize;

		ds::Momentum mMomentum;
		bool		 mTouching;

		bool mAutoSendToFront;

		float	  mAnimDuration;
		bool	  mAnimating;
		bool	  mEnableAfterAnimating;
		bool	  mRemoving; // the panel is on it's last animation
		ci::Rectf mBoundingArea;

		std::function<void()> mLayoutCallback;
		std::function<void()> mPositionUpdateCallback;

	  private:
		// This method is intentially private.
		// The reason is that your layout code should go into layout(), and may be called
		// at any time, not just when the view changes size. So your layout code should be lightweight
		// Having this be protected or public could lead to duplicate layout codepaths, making debugging tricky
		// If you need something to update with the size changes, override onLayout() and put your code there.
		virtual void onSizeChanged();
	};

}} // namespace ds::ui

#endif // UI_PANEL_PANEL_BASE
