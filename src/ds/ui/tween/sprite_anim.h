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
	static const SpriteAnim<ci::vec3>&		ANIM_POSITION();
	static const SpriteAnim<ci::vec3>&		ANIM_SCALE();
	static const SpriteAnim<ci::vec3>&		ANIM_SIZE();
	static const SpriteAnim<ci::vec3>&		ANIM_ROTATION();
	static const SpriteAnim<float>&			ANIM_NORMALIZED();

	void									tweenColor(		const ci::Color&, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);

	void									tweenOpacity(	const float opacity, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenPosition(	const ci::vec3& pos, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenRotation(	const ci::vec3& rot, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenScale(		const ci::vec3& scale, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenSize(		const ci::vec3& size, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);
	void									tweenNormalized(const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr,
															const std::function<void(void)>& updateFn = nullptr);


	/// if any animation is running
	const bool								animationRunning();

	/// individual components running checks
	const bool								getPositionTweenIsRunning();
	const bool								getRotationTweenIsRunning();
	const bool								getScaleTweenIsRunning();
	const bool								getSizeTweenIsRunning();
	const bool								getOpacityTweenIsRunning();
	const bool								getColorTweenIsRunning();
	const bool								getNormalizeTweenIsRunning();

	/// Stops all of the above tweens immediately without calling the finish function. Will leave the sprite in the middle of a tween if a tween was running
	void									animStop();
	void									animPositionStop();
	void									animRotationStop();
	void									animScaleStop();
	void									animSizeStop();
	void									animOpacityStop();
	void									animColorStop();
	void									animNormalizedStop();

	/// Stops any running tweens and set the tween to the end, optionally calling the finish function, optionally recursing into all children
	void									completeAllTweens(const bool callFinishFunctions = false, const bool recursive = false);
	void									completeTweenPosition(const bool callFinishFunction = false);
	void									completeTweenRotation(const bool callFinishFunction = false);
	void									completeTweenScale(const bool callFinishFunction = false);
	void									completeTweenSize(const bool callFinishFunction = false);
	void									completeTweenOpacity(const bool callFinishFunction = false);
	void									completeTweenColor(const bool callFinishFunction = false);
	void									completeTweenNormalized(const bool callFinishFunction = false);

	/// Runs any script set as the animate on script. Optionally runs through any children sprites and runs those as well.
	/// You can also add delta delay so each element runs a bit later than the one before. The first one runs with it's default delay
	void									tweenAnimateOn(const bool recursive = false, const float delay = 0.0f, const float deltaDelay = 0.0f);

	/// Sets the script to use in the above tweenAnimateOn() function
	void									setAnimateOnScript(const std::string& animateOnScript);
	const std::string&						getAnimateOnScript(){ return mAnimateOnScript; }

	/// Sets the targets for animate on. This only applies to fade, grow and slide. The intent here is to make tweenAnimateOn() reliable through multiple calls at any time.
	void									setAnimateOnTargets();
	void									setAnimateOnTargetsIfNeeded();

	/// Reset the animateOn target, which allows you to call a new animateOn to different targets.
	/// setToTarget will automatically set this sprite to it's current targets, making for an easy reset.
	void									clearAnimateOnTargets(const bool recursive = false);

	/** Parse the string as a script to run a few animations.
		Syntax: <type>:<valueX, valueY, valueZ>;
		Special params: 
			easing: see getEasingByString() implementation for details. Same easing applies to all tween types (opacity and position would use the same easing for instance, unfortunately)
			duration: in seconds
			delay: in seconds
			center: change center directly without changing position
			shift: tween the position to the set offset
			slide: tweens the position to the cached destination position, and offsets the start by the supplied values. Cache is created the first time slide, grow or fade is called. Call setAnimateOnTargets() to reset the cached targets.
			grow: tweens the scale to the cached scale, and starts at the supplied value
			fade: tweens the opacity to the cached opacity and starts at the supplied value
		Example: "scale:1, 1, 1; position:100, 200, 300; opacity:1.0; color:0.5, 0.6, 1.0; rotation:0.0, 0.0, 90.0; size:20, 20; easing:inOutBack; duration:1.0; slide:-100; delay:0.5"
		*/
	void									runAnimationScript(const std::string& animScript, const float addedDelay = 0.0f);

	void									runMultiAnimationScripts(const std::vector<std::string> animScripts, const float gapTime, const float addedDelay = 0.0f);
	void									parseMultiScripts(const std::vector<std::string> animScripts, std::vector<float>& durations, std::vector<float>& delays);

	/// Gets the cinder easing function by string value, to support the script running
	static ci::EaseFn						getEasingByString(const std::string& inString);

public:
	ci::Anim<ci::Color>						mAnimColor;
	ci::Anim<float>							mAnimOpacity;
	ci::Anim<ci::vec3>						mAnimPosition;
	ci::Anim<ci::vec3>						mAnimScale;
	ci::Anim<ci::vec3>						mAnimSize;
	ci::Anim<ci::vec3>						mAnimRotation;
	ci::Anim<float>							mAnimNormalized;

	float									getNormalizedTweenValue(){ return mNormalizedTweenValue; }

private:
	Sprite&									mOwner;
	SpriteEngine&							mEngine;

	std::string								mAnimateOnScript;
	bool									mAnimateOnTargetsSet;
	ci::vec3								mAnimateOnScaleTarget;
	ci::vec3								mAnimateOnPositionTarget;
	float									mAnimateOnOpacityTarget;

	float									mNormalizedTweenValue;

	//---- For completing tweens, yaaay ----------------------------//
	ci::TweenRef<ci::vec3>					mInternalPositionCinderTweenRef;
	ci::TweenRef<ci::vec3>					mInternalScaleCinderTweenRef;
	ci::TweenRef<ci::vec3>					mInternalSizeCinderTweenRef;
	ci::TweenRef<ci::vec3>					mInternalRotationCinderTweenRef;
	ci::TweenRef<ci::Color>					mInternalColorCinderTweenRef;
	ci::TweenRef<float>						mInternalOpacityCinderTweenRef;
	ci::TweenRef<float>						mInternalNormalizedCinderTweenRef;

	// Store a CueRef from the cinder timeline to clear the callAfterDelay() function
	// Cleared automatically on destruction
	ci::CueRef			mDelayedCallCueRef;

	// Store multiple CueRefs from the cinder timeline in the case of a multi anim script
	// Cleared automatically on destruction;
	std::vector<ci::CueRef> mMultiDelayedCallCueRefs;

};

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_SPRITEANIM_H_