#pragma once
#ifndef VIEW_TOUCH_BUTTONBEHAVIOUR_H_
#define VIEW_TOUCH_BUTTONBEHAVIOUR_H_

#include <functional>
#include <unordered_map>
#include <cinder/Vector.h>

namespace ds {
namespace ui {
class Sprite;
struct TouchInfo;
} // namespace ui

/**
 * ds::ButtonBehaviour
 * \brief A behaviour object that makes a sprite act like a button.
 */
class ButtonBehaviour {
public:
	ButtonBehaviour(ds::ui::Sprite&);

	void						setOnDownFn(const std::function<void(const ds::ui::TouchInfo&)>&);
	void						setOnEnterFn(const std::function<void(void)>&);
	void						setOnExitFn(const std::function<void(void)>&);
	void						setOnClickFn(const std::function<void(void)>&);
	// This happens when the touch ends but it doesn't result in a click.
	void						setOnUpFn(const std::function<void(void)>&);

	// MACROS. These override the functions.
	// Set the button to scale the owner sprite while it's being pressed
	void						setToScale(	const float pressedScale, const float pressedAnimDuration,
											const std::function<void(void)>& clickFn);
	// Like setToScale, but shrinks by the given number of pixels based on the owner's size
	void						setToScalePixel(const float pixels, const float pressedAnimDuration,
												const std::function<void(void)>& clickFn);

	typedef std::function<bool(const ci::Vec3f&)>
								TouchInsideCheckFunction;
	void						setTouchInsideCheckFunction( const TouchInsideCheckFunction &f ) { mTouchInsideCheckFunction = f; }

	void						enable();
	void						disable();

private:
	void						handleTouch(const ds::ui::TouchInfo&);
	bool						ownerContains(const ci::Vec3f& point) const;

	ds::ui::Sprite&				mOwner;

	static const enum State { STATE_EMPTY, STATE_INSIDE, STATE_OUTSIDE };
	State						mState;

	std::function<void(const ds::ui::TouchInfo&)>
								mOnDownFn;
	std::function<void(void)>	mOnEnterFn;
	std::function<void(void)>	mOnExitFn;
	std::function<void(void)>	mOnClickFn;
	std::function<void(void)>	mOnUpFn;

	TouchInsideCheckFunction	mTouchInsideCheckFunction;

	// Track the fingers on me
	std::unordered_map<int, ci::Vec3f>
								mFinger;
	bool						mIsSetToScale;
};

} // namespace ds

#endif
