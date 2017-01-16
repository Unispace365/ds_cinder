#include "tap_action.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/touch/touch_event.h>

#include <cinder/Rand.h>
#include <cinder/app/TouchEvent.h>

namespace ds {
namespace debug {

/**
 * \class ds::TapActionFactory
 */
float TapActionFactory::getLimit() const {
	return ci::randFloat(0.01f, 0.5f);
}

int TapActionFactory::getNumberOfFingers() const {
	return 1;
	// Our taps are considered 1-finger, so dunno why you would ever want a 0 or 4 finger tap since that wouldnt do shit
	//	return ci::randInt(0, 5);
}

BaseAction* TapActionFactory::build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const {
	return new TapAction(freeList, engine, frame);
}

/**
 * \class ds::TapAction
 */
TapAction::TapAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame)
	: BaseAction(freeList, engine, frame)
{
}

TapAction::~TapAction(){
}

bool TapAction::update(float dt){
	BaseAction::update(dt);

	if(mTotal >= mLimit)
	{
		std::vector<ci::app::TouchEvent::Touch> touches;
		for(int i = 0; i < mNumberOfFingers; ++i){
			touches.push_back(ci::app::TouchEvent::Touch(mTouchPos[i], mTouchPos[i], mInUseList[i], dt, nullptr));
		}
		mEngine.injectTouchesEnded(ds::ui::TouchEvent(mEngine.getWindow(), touches));
		return true;
	}

	return false;
}

void TapAction::setup(float limit, int numberOfFingers)
{
	BaseAction::setup(limit, numberOfFingers);

	std::vector<ci::app::TouchEvent::Touch> touches;

	mTouchPos.reserve(mInUseList.size());
	for(auto it = mInUseList.begin(), it2 = mInUseList.end(); it != it2; ++it){
		ci::vec2 touchPos = ci::vec2(mFrame.getX1() + ci::randFloat(0.0f, mFrame.getWidth()), mFrame.getY1() + ci::randFloat(0.0f, mFrame.getHeight()));
		mTouchPos.push_back(touchPos);
		touches.push_back(ci::app::TouchEvent::Touch(touchPos, touchPos, *it, 0.0, nullptr));
	}
	mEngine.injectTouchesBegin(ds::ui::TouchEvent(mEngine.getWindow(), touches));
}

} // namespace debug
} // namespace ds