#pragma once
#ifndef DS_UI_TWEEN_SPRITEANIM_H_
#define DS_UI_TWEEN_SPRITEANIM_H_

#include <cinder/Color.h>
#include <cinder/Easing.h>
#include <cinder/Tween.h>
#include <cinder/Vector.h>

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;

/**
 * \class ds::ui::SpriteAnim
 * A utility to class to provide animation access to a single
 * sprite property.
 *
 * To construct a sprite anim, you first need a property to
 * animate (generally a ci::Anim<> that exists in the sprite),
 * then you need to make this class and give access to:
 * 1. The property being animated
 * 2. A getter on the initial value of the property
 * 3. A setter to assign the current property value
 */
template<typename T>
class SpriteAnim {
  public:
	SpriteAnim( // Provide access to the Anim<> object that holds the value we will animate
				const std::function<ci::Anim<T>&(Sprite&)>& getAnim,
				// Answer the current value of the property we will animate
				const std::function<T(Sprite&)>& getStartValue,
				// Assign the new property value
				const std::function<void(const T&, Sprite&)>& assignValue)
		: mGetAnim(getAnim)
		, mGetStartValue(getStartValue)
		, mAssignValue(assignValue)
	{
	}

	ci::Anim<T>&        getAnim(Sprite& s) const        { return mGetAnim(s); }
	const T             getStartValue(Sprite& s) const  { return mGetStartValue(s); }
	const std::function<void(const T&, Sprite&)>&
						getAssignValue() const          { return mAssignValue; }

  private:
	const std::function<ci::Anim<T>&(Sprite&)>          mGetAnim;
	const std::function<T(Sprite&)>                     mGetStartValue;
	const std::function<void(const T&, Sprite&)>        mAssignValue;
};

/**
 * \class ds::ui::SpriteAnimatable
 * Provide conveniences for the common properties that can
 * be animated on a sprite.
 */
class SpriteAnimatable {
public:
	SpriteAnimatable(Sprite&, SpriteEngine&);
	virtual ~SpriteAnimatable();

	static const SpriteAnim<ci::Color>&		ANIM_COLOR();
	static const SpriteAnim<float>&			ANIM_OPACITY();
	static const SpriteAnim<ci::Vec3f>&		ANIM_POSITION();
	static const SpriteAnim<ci::Vec3f>&		ANIM_SCALE();
	static const SpriteAnim<ci::Vec3f>&		ANIM_SIZE();
	static const SpriteAnim<ci::Vec3f>&		ANIM_ROTATION();

	void									tweenColor(		const ci::Color&, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenOpacity(	const float opacity, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenPosition(	const ci::Vec3f& pos, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenRotation(	const ci::Vec3f& rot, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenScale(		const ci::Vec3f& scale, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenSize(		const ci::Vec3f& size, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									animStop();

	/// Runs any script set as the animate on script. Optionally runs through any children sprites and runs those as well.
	/// You can also add delta delay so each element runs a bit later than the one before. The first one runs with it's default delay
	void									tweenAnimateOn(const bool recursive = false, const float delay = 0.0f, const float deltaDelay = 0.0f);

	/// Sets the script to use in the above tweenAnimateOn() function
	void									setAnimateOnScript(const std::string& animateOnScript);

	/** Parse the string as a script to run a few animations.
		Syntax: <type>:<valueX, valueY, valueZ>;
		Special params: 
			easing: see getEasingByString() implementation for details. Same easing applies to all tween types (opacity and position would use the same easing for instance, unfortunately)
			duration: in seconds
			delay: in seconds
			slide: tweens the position to the current position, and offsets the start by the supplied values
			grow: tweens the scale to the current scale, and starts at the supplied value
			fade: tweens the opacity to the current opacity and starts at the supplied value
		Example: "scale:1, 1, 1; position:100, 200, 300; opacity:1.0; color:0.5, 0.6, 1.0; rotation:0.0, 0.0, 90.0; size:20, 20; easing:inOutBack; duration:1.0; slide:-100; delay:0.5"
		*/
	void									runAnimationScript(const std::string& animScript, const float addedDelay = 0.0f);

	/// Gets the cinder easing function by string value, to support the script running
	static ci::EaseFn						getEasingByString(const std::string& inString);

public:
	ci::Anim<ci::Color>						mAnimColor;
	ci::Anim<float>							mAnimOpacity;
	ci::Anim<ci::Vec3f>						mAnimPosition;
	ci::Anim<ci::Vec3f>						mAnimScale;
	ci::Anim<ci::Vec3f>						mAnimSize;
	ci::Anim<ci::Vec3f>						mAnimRotation;

private:
	Sprite&									mOwner;
	SpriteEngine&							mEngine;
	std::string								mAnimateOnScript;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_SPRITEANIM_H_