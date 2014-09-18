#ifndef _DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_
#define _DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_

#include "sprite_anim.h"

#include <ds/app/event_client.h>
#include "ds/app/event_notifier.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "sprite_tween_events.h"
#include "tweenline.h"

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
				[key,	 this](ds::ui::Sprite& s)->ci::Anim<PropType>& { return boost::any_cast<cinder::Anim<PropType> &>(mAnimatedProperties[key].second); },
				[getter, key, this](ds::ui::Sprite& s)->PropType { return getterWrapper<PropType>(key, &getter); },
				[setter, key, this](const PropType& v, ds::ui::Sprite& s) { setterWrapper<PropType>(key, &setter, v); }), ci::Anim<PropType>())));
			mEventNotifier.notify(TweenEventAdded(key));
			return true;
		}
		else return false;
	}
	
	// Tweens a member of a sprite inherited class to previously added via SpriteAnimatableBehaviors::add API call
	template<typename PropType>
	void									tween(const std::string& key, const PropType& val, const float duration = 1.0f, const float delay = 0.0f,
													const ci::EaseFn& ease = ci::easeNone, const std::function<void(void)>& finishFn = [](){}) {
		mEventNotifier.notify(TweenEventStart(key));
		if (contains(key)) {
			boost::any_cast<cinder::Anim<PropType> &>(mAnimatedProperties[key].second).stop();
			mOwner.getEngine().getTweenline().apply(mOwner, boost::any_cast<const ds::ui::SpriteAnim<PropType> &>(mAnimatedProperties[key].first), val, duration, ease, [finishFn, key, this](){mEventNotifier.notify(TweenEventEnd(key)); finishFn(); }, delay);
		}
	}
	
	// Checks if an animatble behavior exists with a given key
	bool									contains(const std::string& key);
	
	// Removes an animatble behavior with a given key
	void									remove(const std::string& key);

	// returns the communication channel name used by the current instance.
	const std::string&						getChannelName() const;

	// Callback setters
	void									setAddFn(std::function<void(const std::string&)>);
	void									setRemoveFn(std::function<void(const std::string&)>);
	void									setStartFn(std::function<void(const std::string&)>);
	void									setEndFn(std::function<void(const std::string&)>);
	void									setChangeFn(std::function<void(const std::string&)>);

private:
	// NOTE: first boost::any will always be cinder::Anim<...>
	//       second boost::any will always be ds::ui::SpriteAnim<...>
	std::unordered_map < std::string, std::pair<boost::any, boost::any> >
											mAnimatedProperties;
	Sprite&									mOwner;
	const std::string						mEventChannelName;
	ds::EventNotifier&						mEventNotifier;
	ds::EventClient							mEventClient;

	// Wrappers
	template<typename T>
	T										getterWrapper(const std::string& key, const std::function<T()>* const raw) {
		return (*raw)();
	}
	template<typename T>
	void									setterWrapper(const std::string& key, const std::function<void(const T&)>* const raw, const T& val) {
		mEventNotifier.notify(TweenEventChange(key));
		return (*raw)(val);
	}

	// Event handler
	void									onEventNotify(const ds::Event* e);

	// Event notified functions
	std::function<void(const std::string& key)>
											mTweenStart;
	std::function<void(const std::string& key)>
											mTweenEnd;
	std::function < void(const std::string& key) >
											mTweenChange;
	std::function<void(const std::string& key)>
											mTweenAdded;
	std::function<void(const std::string& key)>
											mTweenRemoved;
};

} //!namespace ui
} //!namespace ds

#endif //!_DS_UI_SPRITE_ANIMATABLE_BEHAVIOR_H_