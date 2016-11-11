#include "base_panel.h"

#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/params/update_params.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

BasePanel::BasePanel(ds::ui::SpriteEngine& engine)
	: ds::ui::Sprite(engine)
	, mContentAspectRatio(1.0f)
	, mMomentum(engine)
	, mAnimDuration(0.35f)
	, mLeftPad(0.0f)
	, mTopPad(0.0f)
	, mRightPad(0.0f)
	, mBottomPad(0.0f)
	, mAbsMinSize(300.0f, 300.0f)
	, mAbsMaxSize(20000.0f, 20000.0f)
	, mTouching(false)
	, mAnimating(false)
	, mRemoving(false)
	, mDefaultSize(engine.getWorldWidth(), engine.getWorldHeight())
	, mLayoutCallback(nullptr)
{

	mLayoutFixedAspect = true;

	mMomentum.setMomentumParent(this);
	mMomentum.setMass(8.0f);
	mMomentum.setFriction(0.5f);

	setTouchScaleMode(true);
	setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){handleTouchInfo(ti); });

	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
	enable(true);

	mBoundingArea = ci::Rectf(0.0f, 0.0f, mEngine.getWorldWidth(), mEngine.getWorldHeight());

// 	 	setOpacity(0.5f);
// 	 	setTransparent(false);
// 	 	setColor(ci::Color(0.0f, 0.0f, 0.5f));
}

void BasePanel::handleTouchInfo(const ds::ui::TouchInfo& ti){

	if(ti.mNumberFingers > 0){
		mTouching = true;
	} else {
		mTouching = false;
	}

	if(mAnimating) return;

	activatePanel();

	completeTweenScale();
	completeTweenOpacity();

	if(ti.mPhase == ds::ui::TouchInfo::Added && ti.mFingerIndex == 0){

		animStop();
		mMomentum.deactivate();

	} else if(ti.mPhase == ds::ui::TouchInfo::Moved && ti.mNumberFingers > 1){
		float conWidth = getWidth() - mLeftPad - mRightPad;
		float conHeight = conWidth / mContentAspectRatio;

		if(conWidth < mMinSize.x){
			conWidth = mMinSize.x;
		} else if(conWidth > mMaxSize.x){
			conWidth = mMaxSize.x;
		}
		if(conHeight < mMinSize.y){
			conHeight = mMinSize.y;
			conWidth = mContentAspectRatio * conHeight;
		} else if(conHeight > mMaxSize.y){
			conHeight = mMaxSize.y;
			conWidth = mContentAspectRatio * conHeight;
		}

		setViewerSize(conWidth, conHeight);
	} else if(ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0){
		mMomentum.activate();
	}
}

void BasePanel::updateServer(const ds::UpdateParams &updateParams){

	ds::ui::Sprite::updateServer(updateParams);
	if(mMomentum.recentlyMoved() && !mAnimating){
		checkBounds();
	}
}

void BasePanel::layout(){
	onLayout();
	if(mLayoutCallback){
		mLayoutCallback();
	}
}

void BasePanel::onSizeChanged() {
	layout();
}


void BasePanel::setViewerSize(float contentWidth, float contentHeight){
	if(contentWidth < mMinSize.x || contentHeight < mMinSize.y){
		contentWidth = mMinSize.x;
		contentHeight = mMinSize.y;
	} else if(contentWidth > mMaxSize.x || contentHeight > mMaxSize.y){
		contentWidth = mMaxSize.x;
		contentHeight = mMaxSize.y;
	}

	const float nw = contentWidth + mLeftPad + mRightPad, nh = contentHeight + mTopPad + mBottomPad;

	setSize(nw, nh);
}

void BasePanel::setViewerSize(const ci::Vec2f newContentSize){
	setViewerSize(newContentSize.x, newContentSize.y);
}

void BasePanel::setViewerWidth(const float contentWidth){
	setViewerSize(contentWidth, contentWidth / mContentAspectRatio);
}

void BasePanel::setViewerHeight(const float contentHeight){
	setViewerSize(contentHeight * mContentAspectRatio, contentHeight);
}

void BasePanel::setAbsoluteSizeLimits(const ci::Vec2f& absMin, const ci::Vec2f& absMax) {
	mAbsMinSize = absMin; mAbsMaxSize = absMax;
}

void BasePanel::setSizeLimits(){
	ci::Vec2f	panelDefaultSize = mDefaultSize;
	const float			cw = getWidth() != 0 ? getWidth() : 1.0f, ch = getHeight() != 0 ? getHeight() : 1.0f, aspect = cw / ch;
	ci::Vec2f			minSize = ci::Vec2f(cw, ch),
						defaultSize = ci::Vec2f(cw, ch),
						maxSize = ci::Vec2f(cw * 10.0f, ch * 10.0f);
	const ci::Vec2f		absMinSize = mAbsMinSize,
						idealDefault = ci::Vec2f(panelDefaultSize.x, panelDefaultSize.y),
						absMaxSize = mAbsMaxSize;
	const float			absMinArea = absMinSize.x * absMinSize.y;
	const float			absMaxArea = absMaxSize.x * absMaxSize.y;
	const float			idealDefaultArea = idealDefault.x * idealDefault.y;


	// make the min size the absolute min size
	minSize.y = sqrt(absMinArea / aspect);
	minSize.x = minSize.y * aspect;
	defaultSize.y = sqrt(idealDefaultArea / aspect);
	defaultSize.x = defaultSize.y * aspect;
	maxSize.y = sqrt(absMaxArea / aspect);
	maxSize.x = maxSize.y * aspect;


	if(minSize.y > absMinSize.y){
		minSize.y = absMinSize.y;
		minSize.x = minSize.y * aspect;
	}
	if(minSize.x > absMinSize.x){
		minSize.x = absMinSize.x;
		minSize.y = minSize.x / aspect;
	}

	// Verify default size isn't bigger than the ideal default size or smaller than the min size.
	if(defaultSize.x < minSize.x){
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}
	if(defaultSize.y < minSize.y){
		defaultSize.y = idealDefault.y;
		defaultSize.x = defaultSize.y * aspect;
	}
	if(defaultSize.x > idealDefault.x){
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}
	if(defaultSize.y > idealDefault.y){
		defaultSize.y = idealDefault.y;
		defaultSize.x = defaultSize.y * aspect;
	}
	if(defaultSize.x > idealDefault.x){
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}


	if(maxSize.x > absMaxSize.x){
		maxSize.x = absMaxSize.x;
		maxSize.y = maxSize.x / aspect;
	}
	if(maxSize.y > absMaxSize.y){
		maxSize.y = absMaxSize.y;
		maxSize.x = maxSize.y * aspect;
	}
	if(maxSize.x > absMaxSize.x){
		maxSize.x = absMaxSize.x;
		maxSize.y = maxSize.x / aspect;
	}
	//cout << "content view size limits: " << minSize.x << " " << minSize.y << " " << defaultSize.x << " " << defaultSize.y << " " << maxSize.x << " " << maxSize.y << endl;
	mMinSize = minSize;
	mDefaultSize = defaultSize;
	mMaxSize = maxSize;
}

void BasePanel::checkBounds(const bool immediate) {

	if(mAnimating && !immediate) return;

	const float thisWidth = getWidth();
	const float thisHeight = getHeight();

	const float anchorX = getCenter().x * thisWidth;
	const float anchorY = getCenter().y * thisHeight;

	const float thisX = getPosition().x - anchorX;
	const float thisY = getPosition().y - anchorY;

	const float worldL = mBoundingArea.getX1();
	const float worldR = mBoundingArea.getX2();
	const float worldW = mBoundingArea.getWidth();
	const float worldT = mBoundingArea.getY1();
	const float worldB = mBoundingArea.getY2();
	const float worldH = mBoundingArea.getHeight();

	float destinationX = thisX;
	float destinationY = thisY;

	if(thisWidth < worldW){
		if(thisX < worldL){
			destinationX = worldL;
		} else if(thisX > worldR - thisWidth){
			destinationX = worldR - thisWidth;
		}
	} else {
		if(thisX < worldR - thisWidth){
			destinationX = worldR - thisWidth;
		} else if(thisX > worldL){
			destinationX = worldL;
		}
	}

	if(thisHeight < worldH){
		if(thisY < worldT){
			destinationY = worldT;
		} else if(thisY > worldB - thisHeight){
			destinationY = worldB - thisHeight;
		}
	} else {
		if(thisY < worldB - thisHeight){
			destinationY = worldB - thisHeight;
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
		setPosition(floorf(destinationX), floorf(destinationY));
	} else {
		tweenPosition(ci::Vec3f(destinationX, destinationY, 0.0f), mAnimDuration, 0.0f, ci::EaseOutQuint());
	}
}

void BasePanel::userInputReceived() {
	ds::ui::Sprite::userInputReceived();
	activatePanel();
}

void BasePanel::activatePanel() {
	sendToFront();
	onPanelActivated();
}

void BasePanel::tweenStarted(){
	mMomentum.deactivate();
	enable(false);
	mAnimating = true;
}

void BasePanel::tweenEnded(){
	enable(true);
	mAnimating = false;
	checkBounds();
	layout(); // sometimes tweens are happening and not laying out properly, so just to be sure
}

void BasePanel::animateToDefaultSize(){
	animateSizeTo(mDefaultSize);
}

void BasePanel::animateSizeTo(const ci::Vec2f newContentSize){
	ci::Vec3f destSize = ci::Vec3f(newContentSize.x + mLeftPad + mRightPad, newContentSize.y + mTopPad + mBottomPad, 0.0f);
	tweenStarted();
	tweenSize(destSize, mAnimDuration, 0.0f, ci::EaseInOutQuad(), [this](){ tweenEnded(); });
}

void BasePanel::animateWidthTo(const float newWidth){
	animateSizeTo(ci::Vec2f(newWidth, newWidth / mContentAspectRatio));
}

void BasePanel::animateHeightTo(const float newHeight){
	animateSizeTo(ci::Vec2f(newHeight * mContentAspectRatio, newHeight));
}

void BasePanel::setLayoutCallback(std::function<void()> layoutCallback){
	mLayoutCallback = layoutCallback;
}

} // namespace ui
} // namespace ds