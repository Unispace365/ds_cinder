#include "stdafx.h"

#include "touch_debug.h"

#include <ds/app/engine/engine.h>
#include <ds/ui/touch/touch_event.h>

#include <cinder/CinderMath.h>
#include <cinder/Rand.h>

namespace ds {
namespace ui {

TouchDebug::TouchDebug(ds::Engine& enginey)
	: mEngine(enginey)
	, mTouchId(100)
	, mNumberOfReplicants(5)
	, mReplicating(false)
	, mTwoTouching(false)
	, mDropTouched(false)
	, mFiveTouchRadius(40.0f)
{
}

void TouchDebug::mouseDown(const ci::app::MouseEvent& e) {

	mReplicating = false;

	if(e.isAltDown()) {
		if(!mDropTouched) {
			mEngine.mouseTouchBegin(e, mTouchId + 2);
			mDropTouched = true;
		} else {
			mEngine.mouseTouchEnded(e, mTouchId + 2);
			mDropTouched = false;
		}
	} else if(e.isShiftDown()) {
		mReplicating = true;
		replicate(e, ds::ui::TouchInfo::Added);

	} else if(e.isControlDown()) {
			mTwoTouching = true;
			mTwoTouchDown = e.getPos() + ci::ivec2(mEngine.getMinTouchDistance(), mEngine.getMinTouchDistance());

			mEngine.mouseTouchBegin(e, mTouchId);

			int deltaX = e.getPos().x - mTwoTouchDown.x;
			int deltaY = e.getPos().y - mTwoTouchDown.y;
			ci::app::MouseEvent mouseTwo = ci::app::MouseEvent(e.getWindow(), 0, mTwoTouchDown.x - deltaX, mTwoTouchDown.y - deltaY, e.getNativeModifiers(), e.getWheelIncrement(), e.getNativeModifiers());
			mEngine.mouseTouchBegin(mouseTwo, mTouchId + 1);
		
	} else {
		mEngine.mouseTouchBegin(e, mTouchId);
	}
}

void TouchDebug::mouseDrag(const ci::app::MouseEvent& e) {
	if(mReplicating) {
		replicate(e, ds::ui::TouchInfo::Moved);
	} else if(mTwoTouching) {

		mEngine.mouseTouchMoved(e, mTouchId);

		int deltaX = e.getPos().x - mTwoTouchDown.x;
		int deltaY = e.getPos().y - mTwoTouchDown.y;
		ci::app::MouseEvent mouseTwo = ci::app::MouseEvent(e.getWindow(), 0, mTwoTouchDown.x - deltaX, mTwoTouchDown.y - deltaY, e.getNativeModifiers(), e.getWheelIncrement(), e.getNativeModifiers());
		mEngine.mouseTouchMoved(mouseTwo, mTouchId + 1);

	} else {
		mEngine.mouseTouchMoved(e, mTouchId);
	}
}

void TouchDebug::mouseUp(const ci::app::MouseEvent& e) {
	if(mReplicating) {
		replicate(e, ds::ui::TouchInfo::Removed);
	} else {
		if(mTwoTouching) {
			mEngine.mouseTouchEnded(e, mTouchId + 1);
			mTwoTouching = false;
		}
		mEngine.mouseTouchEnded(e, mTouchId);
	}
}

void TouchDebug::replicate(const ci::app::MouseEvent& eventy, ds::ui::TouchInfo::Phase p) {
	ci::app::MouseEvent alteredMouse = mEngine.alteredMouseEvent(eventy);
	std::vector<ci::app::TouchEvent::Touch> touches;

	static float startAngle = 0.0f;
	startAngle += 1.0f;
	float angley = startAngle;
	float deltaAngley = 360.0f / mNumberOfReplicants;
	float radiusy = mFiveTouchRadius;

	for(int k = 0; k < mNumberOfReplicants; ++k) {
		ci::vec2 thisPos = ci::vec2((float)alteredMouse.getPos().x + radiusy * cos(ci::toRadians(angley)), (float)alteredMouse.getPos().y + radiusy * sin(ci::toRadians(angley)));
		touches.push_back(ci::app::TouchEvent::Touch(thisPos, thisPos, mTouchId + k, 0.0, nullptr));
		angley += deltaAngley;
	}

	ds::ui::TouchEvent te = ds::ui::TouchEvent(mEngine.getWindow(), touches, true);
	if(p == ds::ui::TouchInfo::Added) {
		mEngine.injectTouchesBegin(te);
	} else if(p == ds::ui::TouchInfo::Moved) {
		mEngine.injectTouchesMoved(te);
	} else if(p == ds::ui::TouchInfo::Removed) {
		mEngine.injectTouchesEnded(te);
	}
}


} // namespace ui
} // namespace ds
