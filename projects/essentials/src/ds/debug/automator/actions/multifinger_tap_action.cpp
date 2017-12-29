#include "stdafx.h"

#include "multifinger_tap_action.h"
#include <ds/ui/sprite/sprite_engine.h>
#include <cinder/Rand.h>
#include <cinder/app/TouchEvent.h>
#include <ds/ui/touch/touch_event.h>

namespace ds{
namespace debug{

/**
 * \class ds::MultiTapActionFactory
 */
float MultiTapActionFactory::getLimit() const {
	return ci::randFloat(0.01f, 0.5f);
}

int MultiTapActionFactory::getNumberOfFingers() const {
	return ci::randInt(1, 10);
}

BaseAction* MultiTapActionFactory::build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const {
	return new MultiTapAction(freeList, engine, frame);
}

/**
 * \class ds::MultiTapAction
 */
MultiTapAction::MultiTapAction(std::vector<int> &freeList, ds::ui::SpriteEngine &world, const ci::Rectf& frame)
	: BaseAction(freeList, world, frame)
{
}

MultiTapAction::~MultiTapAction(){
}

bool MultiTapAction::update(float dt){
	BaseAction::update(dt);

	if(mTotal >= mLimit)
	{
		std::vector<ci::app::TouchEvent::Touch> touches;
		for(int i = 0; i < mNumberOfFingers; ++i){
			touches.push_back(ci::app::TouchEvent::Touch(mTouchPos[i], mTouchPos[i], mInUseList[i], dt, nullptr));
		}
		mEngine.injectTouchesEnded(ds::ui::TouchEvent(mEngine.getWindow(), touches, true));
		return true;
	}

	return false;
}

void MultiTapAction::setup(float limit, int numberOfFingers){
	BaseAction::setup(limit, numberOfFingers);

	mTouchPos.reserve(mInUseList.size());
	float radius = 20.0f;
	ci::vec2 touchPos = ci::vec2(mFrame.getX1() + ci::randFloat(0.0f, mFrame.getWidth()), mFrame.getY1() + ci::randFloat(0.0f, mFrame.getHeight()));

	float step = (2.0f*(float)M_PI) / mNumberOfFingers;
	float angle = (float)M_PI;

	std::vector<ci::app::TouchEvent::Touch> touches;
	for(auto it = mInUseList.begin(), it2 = mInUseList.end(); it != it2; ++it)	{
		ci::vec2 nTouchPos = touchPos + ci::vec2(cos(angle)*radius - sin(angle)*radius, sin(angle)*radius + cos(angle)*radius);
		mTouchPos.push_back(nTouchPos);
		angle += step;
		touches.push_back(ci::app::TouchEvent::Touch(nTouchPos, nTouchPos, *it, 0.0, nullptr));
	}
	mEngine.injectTouchesBegin(ds::ui::TouchEvent(mEngine.getWindow(), touches, true));
}

} // namespace debug
} // namespace ds
