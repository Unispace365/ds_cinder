#include "stdafx.h"

#include "base_panel.h"

#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/params/update_params.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>


namespace ds::ui {

BasePanel::BasePanel(ds::ui::SpriteEngine& engine)
  : ds::ui::Sprite(engine)
  , mContentAspectRatio(1.0f)
  , mTopPad(0.0f)
  , mLeftPad(0.0f)
  , mRightPad(0.0f)
  , mBottomPad(0.0f)
  , mAbsMinSize(300.0f, 300.0f)
  , mAbsMaxSize(20000.0f, 20000.0f)
  , mDefaultSize(engine.getWorldWidth(), engine.getWorldHeight())
  , mMomentum(engine)
  , mTouching(false)
  , mAutoSendToFront(true)
  , mAnimDuration(0.35f)
  , mAnimating(false)
  , mEnableAfterAnimating(true)
  , mRemoving(false)
  , mLayoutCallback(nullptr)
  , mPositionUpdateCallback(nullptr) {

	mLayoutFixedAspect = true;

	mMomentum.setMomentumParent(this);
	mMomentum.setMass(8.0f);
	mMomentum.setFriction(0.5f);

	setTouchScaleMode(true);
	setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) { handleTouchInfo(ti); });

	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE );
	enable(true);

	mBoundingArea = ci::Rectf(0.0f, 0.0f, mEngine.getWorldWidth(), mEngine.getWorldHeight());

	// 	 	setOpacity(0.5f);
	// 	 	setTransparent(false);
	// 	 	setColor(ci::Color(0.0f, 0.0f, 0.5f));
}

void BasePanel::handleTouchInfo(const ds::ui::TouchInfo& ti) {

	if (ti.mNumberFingers > 0) {
		mTouching = true;
	} else {
		mTouching = false;
	}

	if (mAnimating) return;

	if (mPositionUpdateCallback) mPositionUpdateCallback();

	activatePanel();

	completeTweenScale();
	completeTweenOpacity();

	if (ti.mPhase == ds::ui::TouchInfo::Added && ti.mFingerIndex == 0) {

		animStop();
		mMomentum.deactivate();

	} else if (ti.mPhase == ds::ui::TouchInfo::Moved && ti.mNumberFingers > 1) {
		float conWidth	= getWidth() - mLeftPad - mRightPad;
		float conHeight = conWidth / mContentAspectRatio;

		if (conWidth < mMinSize.x) {
			conWidth = mMinSize.x;
		} else if (conWidth > mMaxSize.x) {
			conWidth = mMaxSize.x;
		}
		if (conHeight < mMinSize.y) {
			conHeight = mMinSize.y;
			conWidth  = mContentAspectRatio * conHeight;
		} else if (conHeight > mMaxSize.y) {
			conHeight = mMaxSize.y;
			conWidth  = mContentAspectRatio * conHeight;
		}

		setViewerSize(conWidth, conHeight);
	} else if (ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0) {
		mMomentum.activate();
	}
}

void BasePanel::onUpdateServer(const ds::UpdateParams& updateParams) {
	if (mMomentum.recentlyMoved() && !mAnimating) {
		if (mPositionUpdateCallback) mPositionUpdateCallback();
		checkBounds();
	}
}

void BasePanel::layout() {
	onLayout();
	if (mLayoutCallback) {
		mLayoutCallback();
	}
}

void BasePanel::onSizeChanged() {
	layout();
}


void BasePanel::setViewerSize(float contentWidth, float contentHeight) {
	if (contentWidth < mMinSize.x || contentHeight < mMinSize.y) {
		contentWidth  = mMinSize.x;
		contentHeight = mMinSize.y;
	} else if (contentWidth > mMaxSize.x || contentHeight > mMaxSize.y) {
		contentWidth  = mMaxSize.x;
		contentHeight = mMaxSize.y;
	}

	const float nw = contentWidth + mLeftPad + mRightPad, nh = contentHeight + mTopPad + mBottomPad;

	setSize(nw, nh);
}

void BasePanel::setViewerSize(const ci::vec2 newContentSize) {
	setViewerSize(newContentSize.x, newContentSize.y);
}

void BasePanel::setViewerWidth(const float contentWidth) {
	setViewerSize(contentWidth, contentWidth / mContentAspectRatio);
}

void BasePanel::setViewerHeight(const float contentHeight) {
	setViewerSize(contentHeight * mContentAspectRatio, contentHeight);
}

void BasePanel::setAbsoluteSizeLimits(const ci::vec2& absMin, const ci::vec2& absMax) {
	mAbsMinSize = absMin;
	mAbsMaxSize = absMax;
}

void BasePanel::setSizeLimits() {
	ci::vec2	panelDefaultSize = mDefaultSize;
	float		cw				 = getWidth() != 0 ? getWidth() : 1.0f;
	float		ch				 = getHeight() != 0 ? getHeight() : 1.0f;
	const float aspect			 = cw / ch;
	ci::vec2	minSize = ci::vec2(cw, ch), defaultSize = ci::vec2(cw, ch), maxSize = ci::vec2(cw * 10.0f, ch * 10.0f);
	const ci::vec2 absMinSize = mAbsMinSize, idealDefault = ci::vec2(panelDefaultSize.x, panelDefaultSize.y),
				   absMaxSize	 = mAbsMaxSize;
	const float absMinArea		 = absMinSize.x * absMinSize.y;
	const float absMaxArea		 = absMaxSize.x * absMaxSize.y;
	const float idealDefaultArea = idealDefault.x * idealDefault.y;


	// make the min size the absolute min size
	minSize.y	  = sqrt(absMinArea / aspect);
	minSize.x	  = minSize.y * aspect;
	defaultSize.y = sqrt(idealDefaultArea / aspect);
	defaultSize.x = defaultSize.y * aspect;
	maxSize.y	  = sqrt(absMaxArea / aspect);
	maxSize.x	  = maxSize.y * aspect;


	if (minSize.y > absMinSize.y) {
		minSize.y = absMinSize.y;
		minSize.x = minSize.y * aspect;
	}
	if (minSize.x > absMinSize.x) {
		minSize.x = absMinSize.x;
		minSize.y = minSize.x / aspect;
	}

	// Verify default size isn't bigger than the ideal default size or smaller than the min size.
	if (defaultSize.x < minSize.x) {
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}
	if (defaultSize.y < minSize.y) {
		defaultSize.y = idealDefault.y;
		defaultSize.x = defaultSize.y * aspect;
	}
	if (defaultSize.x > idealDefault.x) {
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}
	if (defaultSize.y > idealDefault.y) {
		defaultSize.y = idealDefault.y;
		defaultSize.x = defaultSize.y * aspect;
	}
	if (defaultSize.x > idealDefault.x) {
		defaultSize.x = idealDefault.x;
		defaultSize.y = defaultSize.x / aspect;
	}


	if (maxSize.x > absMaxSize.x) {
		maxSize.x = absMaxSize.x;
		maxSize.y = maxSize.x / aspect;
	}
	if (maxSize.y > absMaxSize.y) {
		maxSize.y = absMaxSize.y;
		maxSize.x = maxSize.y * aspect;
	}
	if (maxSize.x > absMaxSize.x) {
		maxSize.x = absMaxSize.x;
		maxSize.y = maxSize.x / aspect;
	}
	// cout << "content view size limits: " << minSize.x << " " << minSize.y << " " << defaultSize.x << " " <<
	// defaultSize.y << " " << maxSize.x << " " << maxSize.y << endl;
	mMinSize	 = minSize;
	mDefaultSize = defaultSize;
	mMaxSize	 = maxSize;
}

void BasePanel::checkBounds(const bool immediate) {

	if (mPositionUpdateCallback) mPositionUpdateCallback();

	if (mAnimating && !immediate) return;

	// Constrain the bounding box of the sprite to mBoundingArea
	auto		bb		   = getBoundingBox();
	const float thisWidth  = bb.getWidth();
	const float thisHeight = bb.getHeight();
	const float thisX	   = bb.getX1();
	const float thisY	   = bb.getY1();

	// DS_LOG_INFO("BasePanel::checkBounds(): BB size: " << bb);

	const float worldL = mBoundingArea.getX1();
	const float worldR = mBoundingArea.getX2();
	const float worldW = mBoundingArea.getWidth();
	const float worldT = mBoundingArea.getY1();
	const float worldB = mBoundingArea.getY2();
	const float worldH = mBoundingArea.getHeight();

	float destinationX = thisX;
	float destinationY = thisY;

	if (thisWidth < worldW) {
		if (thisX < worldL) {
			destinationX = worldL;
		} else if (thisX > worldR - thisWidth) {
			destinationX = worldR - thisWidth;
		}
	} else {
		if (thisX < worldR - thisWidth) {
			destinationX = worldR - thisWidth;
		} else if (thisX > worldL) {
			destinationX = worldL;
		}
	}

	if (thisHeight < worldH) {
		if (thisY < worldT) {
			destinationY = worldT;
		} else if (thisY > worldB - thisHeight) {
			destinationY = worldB - thisHeight;
		}
	} else {
		if (thisY < worldB - thisHeight) {
			destinationY = worldB - thisHeight;
		} else if (thisY > worldT) {
			destinationY = worldT;
		}
	}

	if (destinationX == thisX && destinationY == thisY) {
		return;
	}

	mMomentum.deactivate();


	// Compute the position of the upper-left corner of the rotated sprite, relative to the bounding box
	const auto normalizeAngle = [](const float degrees) {
		float ret = glm::mod(degrees, 360.0f);
		if (ret < 0) ret += 360.0f;
		return ret;
	};
	const float degrees = normalizeAngle(getRotation().z);

	const int	quadrant = (int)glm::floor(degrees / 90.0f);
	const float radians	 = glm::radians(degrees);
	const float w		 = getScaleWidth();
	const float h		 = getScaleHeight();
	const float W		 = bb.getWidth();
	const float H		 = bb.getHeight();
	const auto	ulPos =
		 (0 == quadrant) ? ci::vec2(h * glm::sin(radians), 0)
						 : ((1 == quadrant) ? ci::vec2(W, -h * glm::cos(radians))
											: ((2 == quadrant) ? ci::vec2(-w * glm::cos(radians), H) : //(3 == quadrant)
												   ci::vec2(0, -w * glm::sin(radians))));

	// DS_LOG_INFO("  BasePanel::checkBounds(): Constrained position: " << destinationX << ", " << destinationY <<
	// ", Angle: " << degrees << " degrees"  ); DS_LOG_INFO("  BasePanel::setBounds(): upper-left position: " <<
	// ulPos );

	// re-apply the anchor offset.
	const auto anchorOffset = ci::vec2(getCenter()) * ci::vec2(getScaleWidth(), getScaleHeight());
	const auto pos = ci::vec3(ci::vec2(destinationX, destinationY) + ulPos + glm::rotate(anchorOffset, radians), 0);

	if (immediate) {
		setPosition(pos);
	} else {
		tweenPosition(pos, mAnimDuration, 0.0f, ci::EaseOutQuint());
	}
}

void BasePanel::userInputReceived() {
	ds::ui::Sprite::userInputReceived();
	activatePanel();
}

void BasePanel::activatePanel() {
	if (mAutoSendToFront) {
		sendToFront();
	}
	onPanelActivated();
}

void BasePanel::setPositionUpdatedCallback(std::function<void()> posUpdateCallback) {
	mPositionUpdateCallback = posUpdateCallback;
}

void BasePanel::tweenStarted() {
	mMomentum.deactivate();

	// check if this should be enabled after animating, and be sure we don't overwrite if this is already animating
	// (and therefore disabled)
	if (!mAnimating) {
		mEnableAfterAnimating = isEnabled();
	}
	enable(false);
	mAnimating = true;
}

void BasePanel::tweenEnded() {
	if (mEnableAfterAnimating) {
		enable(true);
	}
	mAnimating = false;
	checkBounds();
	layout(); // sometimes tweens are happening and not laying out properly, so just to be sure
}


void BasePanel::setAboutToBeRemoved(const bool isRemoving /*= true*/) {
	mRemoving = true;
	onAboutToBeRemoved();
}

void BasePanel::animateToDefaultSize() {
	animateSizeTo(mDefaultSize);
}

void BasePanel::animateSizeTo(const ci::vec2 newContentSize) {
	ci::vec3 destSize =
		ci::vec3(newContentSize.x + mLeftPad + mRightPad, newContentSize.y + mTopPad + mBottomPad, 0.0f);
	tweenStarted();
	tweenSize(destSize, mAnimDuration, 0.0f, ci::EaseInOutQuad(), [this]() { tweenEnded(); });
}

void BasePanel::animateWidthTo(const float newWidth) {
	animateSizeTo(ci::vec2(newWidth, newWidth / mContentAspectRatio));
}

void BasePanel::animateHeightTo(const float newHeight) {
	animateSizeTo(ci::vec2(newHeight * mContentAspectRatio, newHeight));
}

void BasePanel::setLayoutCallback(std::function<void()> layoutCallback) {
	mLayoutCallback = layoutCallback;
}

} // namespace ds::ui
