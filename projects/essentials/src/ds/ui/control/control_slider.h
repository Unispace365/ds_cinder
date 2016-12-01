#pragma once
#ifndef DS_UI_CONTROL_CONTROL_SLIDER
#define DS_UI_CONTROL_CONTROL_SLIDER

#include <ds/ui/sprite/sprite.h>
#include <functional>

namespace ds {
namespace ui {


/// An onscreen slider to control stuff
class ControlSlider : public ds::ui::Sprite {
public:

	// Make a slider with a grey background and lighter grey flexible nub.
	ControlSlider(ds::ui::SpriteEngine& engine, const bool vertical = true, const float uiSize = 7.5f, const float slideLength = 100.0f);

	// Someone has touched the scroll bar, and some action should be taken
	// Note that the UI hasn't been updated before this callback is called. 
	// The UI will be updated when you call scrollUpdated()
	// percent is 0.0 to 1.0
	// value is converted to a range between the set min and max (0.0 to 1.0 by default)
	void								setSliderUpdatedCallback(std::function<void(const double sliderPercent, const double sliderValue)> func);

	// A callback that the nub or background have been updated, and you can update any other UI that relates to the nub or background
	void								setVisualUpdateCallback(std::function<void()> func);

	// Call this when the slider has changed, and the nub will be updated
	void								sliderUpdated(const double sliderPercent);

	// Set the value (between min and max) of the slider
	void								setSliderValue(const double sliderValue);

	// Get the sprite in the background. Automatically sized to the size of this scroll bar.
	// Don't release this. 
	// Add any UI for the background to this sprite, or modify the visible parameters (transparent, color, opacity, etc)
	ds::ui::Sprite*						getBackgroundSprite();

	// Get the sprite that indicates where the scroll is. Automatically sized to the percentage visible.
	// Don't release this. 
	// Add any UI for the nub to this sprite or modify the visible parameters
	ds::ui::Sprite*						getNubSprite();

	// Adds space between this (the touch handler) and the background (the visible background)
	void								setTouchPadding(const float touchPadding);

	/// Sets the bounds returned by the slider
	void								setSliderLimits(const double minValue, const double maxValue);

	typedef enum { kSliderTypeLinear = 0, kSliderTypeQuadratic } SliderInterpolation;
	void								setSliderInterpolation(SliderInterpolation interp){ mSliderInterpolation = interp; }

protected:
	void								handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);
	virtual void						onSizeChanged();
	virtual void						layout();

	void								updateNubPosition();

	bool								mVertical;
	std::function<void(const double, const double)>	mSliderUpdatedCallback;
	std::function<void()>				mVisualUpdateCallback;

	ds::ui::Sprite*						mBackground;
	ds::ui::Sprite*						mNub;

	float								mTouchPadding;
	double								mSliderPercent;
	double								mMinValue;
	double								mMaxValue;

	SliderInterpolation					mSliderInterpolation;
};

} // namespace ui
} // namespace ds
#endif