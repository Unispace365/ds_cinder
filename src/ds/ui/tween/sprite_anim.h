#pragma once
#ifndef DS_UI_TWEEN_SPRITEANIM_H_
#define DS_UI_TWEEN_SPRITEANIM_H_

#include <cinder/Color.h>
#include <cinder/Easing.h>
#include <cinder/Tween.h>
#include <cinder/Vector.h>
#include <boost/any.hpp>
#include <unordered_map>

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
* \class ds::ui::SpriteAnimatableBehaviors
* A utility to class to provide animation access to arbitrary
* sprite members.
* 
* \author Sepehr Laal
*
* To use this class, make it a member of your sprite then
* use add method to add your sprites members to it like:
*     mBehavior.add<memberType>( "member", getter, setter );
* you can then use tween API to make that member animated with your
* setter and getter provided:
*     mBehavior.tween( "member", ... );
* the rest of the tween API call arguments is identical to Sprite's
* legacy tweenXX API's
*/
class SpriteAnimatableBehaviors {
public:
	SpriteAnimatableBehaviors(Sprite& s) : mOwner(s) {};
	
	// Adds a member of a sprite inherited class to the list of animatable behaviors
	// use const std::string& key argument to later reference it via "tween" or "remove" API calls
	template<typename PropType>
	bool									add(const std::string& key, std::function<PropType()> getter, std::function<void(const PropType&)> setter) {
		if (!contains(key)) {
			mAnimatedProperties.insert(std::make_pair(key, std::make_pair(ds::ui::SpriteAnim<PropType>(
				[key, this](ds::ui::Sprite& s)->ci::Anim<PropType>& { return boost::any_cast<cinder::Anim<PropType> &>(mAnimatedProperties[key].second); },
				[getter](ds::ui::Sprite& s)->PropType { return getter(); },
				[setter](const PropType& v, ds::ui::Sprite& s) { setter(v); }), ci::Anim<PropType>())));
			return true;
		}
		else return false;
	}
	
	// Tweens a member of a sprite inherited class to previously added via SpriteAnimatableBehaviors::add API call
	template<typename PropType>
	void									tween(const std::string& key, const PropType& val, const float duration = 1.0f, const float delay = 0.0f,
													const ci::EaseFn& ease = ci::easeNone, const std::function<void(void)>& finishFn = [](){}) {
		if (contains(key)) {
			boost::any_cast<cinder::Anim<PropType> &>(mAnimatedProperties[key].second).stop();
			mOwner.getEngine().getTweenline().apply(mOwner, boost::any_cast<const ds::ui::SpriteAnim<PropType> &>(mAnimatedProperties[key].first), val, duration, ease, finishFn, delay);
		}
	}
	
	// Checks if an animatble behavior exists with a given key
	bool									contains(const std::string& key) { return mAnimatedProperties.find(key) != mAnimatedProperties.end(); };
	
	// Removes an animatble behavior with a given key
	void									remove(const std::string& key) { if (contains(key)) mAnimatedProperties.erase(key); };

private:
	// NOTE: first boost::any will always be cinder::Anim<...>
	//       second boost::any will always be ds::ui::SpriteAnim<...>
	std::unordered_map < std::string, std::pair<boost::any, boost::any> >
											mAnimatedProperties;
	Sprite&									mOwner;
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
															const std::function<void(void)>& finishFn = nullptr);
	void									tweenOpacity(	const float opacity, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr);
	void									tweenPosition(	const ci::Vec3f& pos, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr);
	void									tweenRotation(	const ci::Vec3f& rot, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr);
	void									tweenScale(		const ci::Vec3f& scale, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr);
	void									tweenSize(		const ci::Vec3f& size, const float duration = 1.0f, const float delay = 0.0f,
															const ci::EaseFn& = ci::easeNone,
															const std::function<void(void)>& finishFn = nullptr);
	void									animStop();

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
};

} // namespace ui
} // namespace ds

#endif // DS_UI_TWEEN_SPRITEANIM_H_