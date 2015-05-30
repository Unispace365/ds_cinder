#include "view_dragger.h"

#include <ds/ui/sprite/sprite_engine.h>
#include "app/globals.h"

namespace example {

/**
 * \class na::ViewDragger
 */
ViewDragger::ViewDragger(Globals& g, ds::ui::Sprite& parent)
		: mParent(parent)
		, mMomentum(parent.getEngine())
		, mIsTouchy(false)
		, mReturnTime(g.getSettingsLayout().getFloat("media_viewer:check_bounds:return_time", 0, 0.6f)) {
	mParent.enable(true);
	mParent.enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mParent.setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){ onTouched(ti);});

	mMomentum.setMomentumParent(&mParent);
	mMomentum.setMass(8.0f);
	mMomentum.setFriction(0.5f);
}

void ViewDragger::updateServer() {
	if (mMomentum.recentlyMoved()) {
		checkBounds(false);
	}
}

bool ViewDragger::hasTouches() const {
	return mIsTouchy;
}

void ViewDragger::startup() {
	checkBounds(true);
}

void ViewDragger::onTouched(const ds::ui::TouchInfo& ti) {
	mParent.sendToFront();
	if (ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0) {
		mIsTouchy = false;
		mMomentum.activate();
	} else {
		mMomentum.deactivate();
		mIsTouchy = true;
	}

	if (ti.mPhase == ds::ui::TouchInfo::Moved && ti.mFingerIndex == 0) {
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

	const float worldW = mParent.getEngine().getWorldWidth();
	const float worldH = mParent.getEngine().getWorldHeight();

	float destinationX = thisX;
	float destinationY = thisY;

	if(thisWidth < worldW){
		if(thisX < 0){
			destinationX = 0;
		} else if(thisX > worldW - thisWidth){
			destinationX = worldW - thisWidth;
		}
	} else {
		if(thisX < worldW - thisWidth){
			destinationX = worldW - thisWidth;
		} else if(thisX > 0){
			destinationX = 0;
		}
	}

	if(thisHeight < worldH){
		if(thisY < 0){
			destinationY = 0;
		} else if(thisY > worldH - thisHeight){
			destinationY = worldH - thisHeight;
		}
	} else {
		if(thisY < worldH - thisHeight){
			destinationY = worldH - thisHeight;
		} else if(thisY > 0){
			destinationY = 0;
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
		mParent.tweenPosition(ci::Vec3f(destinationX, destinationY, 0.0f), mReturnTime, 0.0f, ci::EaseOutQuint());
	}
}

} // namespace example
