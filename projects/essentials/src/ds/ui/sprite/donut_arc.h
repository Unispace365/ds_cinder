#pragma once
#ifndef DS_UI_SPRITE_DONUT_ARC
#define DS_UI_SPRITE_DONUT_ARC

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

/// A Donut-like graph visual that can fill in to a certain percentage
class DonutArc : public ds::ui::Sprite {
public:
	DonutArc(ds::ui::SpriteEngine&, const float theSize = 0.0f);
	~DonutArc();

	/// Sets the pixel width of the donut. Measured from the width of the sprite.
	/// Default is 5.0f pixels
	void		setDonutWidth(const float innerRad);

	/// Set the 0.0-1.0 percentages of fill for the donut
	/// If applyImmediately is true, will draw the new percent on the next draw
	/// If applyImmediately is false, you'll need to call animateOn() to draw the new percentage
	void 		setPercent(const float percent, const bool applyImmediately = true);

	/// The 0.0 - 1.0 fill of the donut
	float		getPercent();

	/// Default is a clockwise fill, use this to fill in the other way
	void 		setAntiClock(const bool isAntiClock);

	/// Get fill direction
	bool		getIsAntiClock();

	/// Sets the draw percentage to 0.0. Use this after setPercent() and before animateOn() to animate the graph from scratch
	void 		resetDrawPercent();

	/// Animate the graph from the current draw percentage to the current overall percent.
	/// You can use this to animate the graph only partially:
	///     setPercent(0.5f);			/// apply the draw percent
	///     setPercent(0.75f, false);	/// dont' apply the draw percent
	///     animateOn(1.0f);			/// animates from 0.5f to 0.75f
	/// Or to animate on fully:
	///		setPercent(0.666f);
	///     resetDrawPercent();
	///     animateOn(1.0f);
	void 		animateOn(const float duration, const float delay = 0.0f, ci::EaseFn theEaseFunction = ci::easeOutQuad);

protected:
	virtual void drawLocalClient();

	float		mDonutWidth;
	float		mPercent;
	float		mDrawPercent;
	float		mIsAntiClock;
};

} // namespace ui
} // namespace ds
#endif