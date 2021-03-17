#include "stdafx.h"

#include "view_dragger.h"

#include <ds/ui/sprite/sprite_engine.h>

namespace ds {

ViewDragger::ViewDragger(ds::ui::Sprite& parent)
	: AutoUpdate(parent.getEngine())
	, mParent(parent)
	, mMomentum(parent.getEngine())
	, mIsTouchy(false)
	, mReturnTime(0.35f) 
{
	mParent.enable(true);
	mParent.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mParent.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){ onTouched(ti); });

	mMomentum.setMomentumParent(&mParent);
	mMomentum.setMass(8.0f);
	mMomentum.setFriction(0.5f);

	mBoundingArea = ci::Rectf(0.0f, 0.0f, mParent.getEngine().getWorldWidth(), mParent.getEngine().getWorldHeight());
}

void ViewDragger::update(const ds::UpdateParams&) {
	if(mMomentum.recentlyMoved()) {
		checkBounds(false);
	}
}

bool ViewDragger::hasTouches() const {
	return mIsTouchy;
}

void ViewDragger::onTouched(const ds::ui::TouchInfo& ti) {
	mParent.sendToFront();
	mParent.animStop();
	if(ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0) {
		mIsTouchy = false;
		mMomentum.activate();
	} else {
		mMomentum.deactivate();
		mIsTouchy = true;
	}

	if(ti.mPhase == ds::ui::TouchInfo::Moved && ti.mFingerIndex == 0) {
		mParent.move(ti.mDeltaPoint);
	}
}

void ViewDragger::checkBounds(const bool immediate) {
	const float thisWidth = mParent.getWidth();
	const float thisHeight = mParent.getHeight();

	const float anchorX = mParent.getCenter().x * thisWidth;
	const float anchorY = mParent.getCenter().y * thisHeight;

	const float thisX = mParent.getPosition().x - anchorX;
	const float thisY = mParent.getPosition().y - anchorY;

	const float worldL = mBoundingArea.getX1();
	const float worldW = mBoundingArea.getX2();
	const float worldT = mBoundingArea.getY1();
	const float worldH = mBoundingArea.getY2();

	float destinationX = thisX;
	float destinationY = thisY;

	if(thisWidth < worldW){
		if(thisX < worldL){
			destinationX = worldL;
		} else if(thisX > worldW - thisWidth){
			destinationX = worldW - thisWidth;
		}
	} else {
		if(thisX < worldW - thisWidth){
			destinationX = worldW - thisWidth;
		} else if(thisX > worldL){
			destinationX = worldL;
		}
	}

	if(thisHeight < worldH){
		if(thisY < worldT){
			destinationY = worldT;
		} else if(thisY > worldH - thisHeight){
			destinationY = worldH - thisHeight;
		}
	} else {
		if(thisY < worldH - thisHeight){
			destinationY = worldH - thisHeight;
		} else if(thisY > worldT){
			destinationY = worldT;
		}
	}

	if(destinationX == thisX && destinationY == thisY){
		return;
	}

	mMomentum.deactivate();

	// re-apply the anchor offset.
	destinationX += anchorX;
	destinationY += anchorY;
	if(immediate){
		mParent.setPosition(floorf(destinationX), floorf(destinationY));
	} else {
		mParent.tweenPosition(ci::vec3(destinationX, destinationY, 0.0f), mReturnTime, 0.0f, ci::EaseOutQuint());
	}
}

} // namespace test
