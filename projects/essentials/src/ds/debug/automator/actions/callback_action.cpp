#include "stdafx.h"

#include "callback_action.h"
#include <ds/ui/sprite/sprite_engine.h>
#include <cinder/Rand.h>
#include <cinder/app/TouchEvent.h>

namespace ds {
namespace debug {

/**
* \class ds::CallbackActionFactory
*/
float CallbackActionFactory::getLimit() const {
	return ci::randFloat(mMinFrequency, mMaxFrequency);
}

int CallbackActionFactory::getNumberOfFingers() const {
	return 1;
}

BaseAction* CallbackActionFactory::build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const {
	return new CallbackAction(freeList, engine, frame, mCallback, mMinFrequency, mMaxFrequency);
}

/**
* \class ds::CallbackAction
*/
CallbackAction::CallbackAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame, const std::function<void(void)>& callback, const float minTime, const float maxTime)
	: BaseAction(freeList, engine, frame)
	, mCallback(callback)
	, mMinTime(minTime)
	, mMaxTime(maxTime)
{
}

bool CallbackAction::update(float dt){
	BaseAction::update(dt);

	if(mTotal >= mLimit){
		if(mCallback) mCallback();
		mTotal = 0.0f;
		mLimit = ci::randFloat(mMinTime, mMaxTime);
		return true;

	}

	return false;
}

void CallbackAction::setup(float limit, int numberOfFingers){
	BaseAction::setup(limit, numberOfFingers);

}

} // namespace debug
} // namespace ds
