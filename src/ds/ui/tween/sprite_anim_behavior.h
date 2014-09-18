#ifndef _DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_
#define _DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_

#include "sprite_anim.h"
#include <boost/any.hpp>
#include <unordered_map>

namespace ds {
namespace ui {
class Sprite;
class SpriteEngine;

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
*
* This also originally was planned to be a spinoff of Flash tween API
*/
class SpriteAnimatableBehaviors {
public:
	SpriteAnimatableBehaviors(Sprite& s);
	virtual ~SpriteAnimatableBehaviors();
	
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
	bool									contains(const std::string& key);
	
	// Removes an animatble behavior with a given key
	void									remove(const std::string& key);

private:
	// NOTE: first boost::any will always be cinder::Anim<...>
	//       second boost::any will always be ds::ui::SpriteAnim<...>
	std::unordered_map < std::string, std::pair<boost::any, boost::any> >
											mAnimatedProperties;
	Sprite&									mOwner;
	const std::string						mEventChannel;
};

} //!namespace ui
} //!namespace ds

#endif //!_DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_