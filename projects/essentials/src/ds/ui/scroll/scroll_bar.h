#pragma once
#ifndef DS_UI_SCROLL_SCROLL_BAR
#define DS_UI_SCROLL_SCROLL_BAR

#include <ds/ui/sprite/gradient_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/touch/momentum.h>
#include <functional>

namespace ds::ui {

class ScrollArea;
class ScrollList;

/// Where scrolls go to get drunk
class ScrollBar : public ds::ui::Sprite {
  public:
	// Make a scroll bar with a grey background and lighter grey flexible nub.
	// Feel free to get the nub and background and change the parameters if you need a different look
	ScrollBar(ds::ui::SpriteEngine& engine, bool vertical = true, float uiWidth = 10.0f, float touchPadding = 20.0f,
			  bool autoHide = true);

	// Someone has touched the scroll bar, and some action should be taken
	// Note that the UI hasn't been updated before this callback is called.
	// The UI will be updated when you call scrollUpdated()
	void setScrollMoveCallback(const std::function<void(float scrollPercent)>& func);

	// A callback that the nub or background have been updated, and you can update any other UI that relates to the
	// nub or background
	void setVisualUpdateCallback(const std::function<void()>& func);

	// Call this when the scroll has changed, and the nub will be updated
	void scrollUpdated(float percentScrolled, float percentVisible);

	// Get the sprite in the background. Automatically sized to the size of this scroll bar.
	// Don't release this.
	// Add any UI for the background to this sprite, or modify the visible parameters (transparent, color, opacity,
	// etc)
	ds::ui::Sprite* getBackgroundSprite() const;

	// Get the sprite that indicates where the scroll is. Automatically sized to the percentage visible.
	// Don't release this.
	// Add any UI for the nub to this sprite or modify the visible parameters
	ds::ui::Sprite* getNubSprite() const;

	// Won't let the nub be any smaller than this when automatically sizing the nub
	void setMinNubSize(float minNub);

	// Allows you to specify a different size for the scroll bar, but it can never be smaller than the minimum nub size
	void setBarSize(float barSize);

	// Adds space between this (the touch handler) and the background (the visible background)
	void setTouchPadding(float touchPadding);

	// Automatically setup the proper callbacks. Use only 1 link function at a time
	void linkScrollArea(ds::ui::ScrollArea* area);
	void linkScrollList(ds::ui::ScrollList* list);

	// Automagically hides the scroll bar if the linked area or list doesn't have any ability to scroll (the content
	// is shorter than the scroll area) Default is true
	void enableAutoHiding(bool autoHide);

  protected:
	void		 handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);
	void		 onSizeChanged() override;
	virtual void layout();

	void updateNubPosition();

	void doAutoHide(bool shouldBeHidden);

	bool					   mVertical;
	std::function<void(float)> mScrollMoveCallback;
	std::function<void()>	   mVisualUpdateCallback;

	ds::ui::Sprite* mBackground;
	ds::ui::Sprite* mNub;

	float mMinNubSize;
	float mBarSize;
	float mTouchPadding;
	float mScrollPercent;
	float mPercentVisible;
	float mTouchOffset;

	bool mAutoHide;
	bool mAutoHidden;
};

} // namespace ds::ui
#endif
