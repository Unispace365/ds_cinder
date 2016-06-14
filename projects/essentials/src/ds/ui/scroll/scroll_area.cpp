#include "scroll_area.h"


namespace ds{
namespace ui{

ScrollArea::ScrollArea(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool vertical)
	: Sprite(engine)
	, mSpriteMomentum(engine)
	, mScroller(nullptr)
	, mScrollable(false)
	, mReturnAnimateTime(0.3f)
	, mWillSnapAfterDelay(false)
	, mTopFade(nullptr)
	, mBottomFade(nullptr)
	, mTopFadeActive(false)
	, mBottomFadeActive(false)
	, mFadeHeight(30.0f)
	, mFadeFullColor(0, 0, 0, 255)
	, mFadeTransColor(0, 0, 0, 0)
	, mScrollUpdatedFunction(nullptr)
	, mTweenCompleteFunction(nullptr)
	, mSnapToPositionFunction(nullptr)
	, mVertical(vertical)
	, mScrollPercent(0.0f)
{

	setSize(startWidth, startHeight);
	mSpriteMomentum.setMass(8.0f);
	mSpriteMomentum.setFriction(0.5f);
	mSpriteMomentum.setMomentumParent(this);

	enable(false);
	setClipping(true);

	mScroller = new Sprite(mEngine);
	if(mScroller){
		mScroller->mExportWithXml = false;
// 		mScroller->setTransparent(false);
// 		mScroller->setColor(ci::Color(0.1f, 0.56f, 0.3f));
		mScroller->setSize(startWidth, startHeight);
		mScroller->enable(true);
		mScroller->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mScroller->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti){handleScrollTouch(bs, ti); });
		mSpriteMomentum.setMomentumParent(mScroller);
		addChildPtr(mScroller);
	}
}

void ScrollArea::setVertical(bool vertical){
	mVertical = vertical;
	// do a layout-type thing to move the fade sprites if needed, and to call checkBounds
	setScrollSize(getWidth(), getHeight());
}

void ScrollArea::setScrollSize(const float newWidth, const float newHeight){
	// onSizeChanged only triggers if the size is the same, so manually force the check bounds/fade stuff in case other things have changed
	if(getWidth() == newWidth && getHeight() == newHeight){
		onSizeChanged();
	} else {
		setSize(newWidth, newHeight);
	}
}

void ScrollArea::onSizeChanged(){
	const float newWidth = getWidth();
	const float newHeight = getHeight();

	if(mTopFade){
		if(mVertical){
			mTopFade->setSize(newWidth, mFadeHeight);
		} else {
			mTopFade->setSize(mFadeHeight, newHeight);
		}
	}
	if(mBottomFade){
		if(mVertical){
			mBottomFade->setSize(newWidth, mFadeHeight);
			mBottomFade->setPosition(0.0f, newHeight - mFadeHeight);
		} else {
			mBottomFade->setSize(mFadeHeight, newHeight);
			mBottomFade->setPosition(newWidth - mFadeHeight, 0.0f);
		}
	}

	if(mScroller){
		mScroller->setSize(0.0f, 0.0f);
		mScroller->sizeToChildBounds();
	}
	checkBounds();
}

void ScrollArea::addSpriteToScroll(ds::ui::Sprite* bs){
	if(mScroller && bs){
		mScroller->addChildPtr(bs);
		mScroller->sizeToChildBounds();
		checkBounds();
	}
}

Sprite* ScrollArea::getSpriteToPassTo(){
	return mScroller;
}

void ScrollArea::checkBounds(){
	bool doTween = true;
	ci::Vec3f tweenDestination = mScroller->getPosition();

	bool snapped = callSnapToPositionCallback(doTween, tweenDestination);

	if(!snapped){
		float scrollWindow(0.0f);
		float scrollerSize(0.0f);

		if(mVertical){
			scrollWindow = getHeight();
			scrollerSize = mScroller->getHeight();
		} else {
			scrollWindow = getWidth();
			scrollerSize = mScroller->getWidth();
		}

		bool canKeepAllScrollerInWindow = false;
		if(scrollerSize <= scrollWindow){
			mScrollable = false;
			canKeepAllScrollerInWindow = true;

			// only allowable position is zero
			tweenDestination.set(0.0f, 0.0f, 0.0f);
		} else {
			float scrollerPos(0.0f);
			if(mVertical){
				scrollerPos = mScroller->getPosition().y;
			} else {
				scrollerPos = mScroller->getPosition().x;
			}

			// find the limits
			float minPos(0.0f);
			float maxPos(0.0f);

			if(canKeepAllScrollerInWindow){
				maxPos = scrollWindow - scrollerSize;
			} else {
				minPos = scrollWindow - scrollerSize;
			}

			if(scrollerPos < minPos){
				// Can't scroll down any more
				if(mVertical){
					tweenDestination.set(0.0f, minPos, 0.0f);
				} else {
					tweenDestination.set(minPos, 0.0f, 0.0f);
				}
			} else if(scrollerPos > maxPos){
				// Can't scroll up any more
				if(mVertical){
					tweenDestination.set(0.0f, maxPos, 0.0f);
				} else {
					tweenDestination.set(maxPos, 0.0f, 0.0f);
				}
			} else {
				// In bounds
				doTween = false;
			}
		}
	}

	if(doTween){
		// respond after a delay, in case this call is cancelled in a swipe callback
		mWillSnapAfterDelay = true;
		callAfterDelay([this, doTween, tweenDestination](){
			mWillSnapAfterDelay = false;
			mSpriteMomentum.deactivate();
			mScroller->tweenPosition(tweenDestination, mReturnAnimateTime, 0.0f, ci::EaseOutQuint(), [this](){ tweenComplete(); }, [this](){ scrollerTweenUpdated(); });
			scrollerUpdated(tweenDestination.xy());
		}, 0.0f);
	} else {
		// nothing special to do here
		mScroller->animStop();
		mScroller->setPosition(tweenDestination);
		scrollerUpdated(tweenDestination.xy());
		tweenComplete();
	}
}

void ScrollArea::updateServer(const ds::UpdateParams& p){
	Sprite::updateServer(p);
	if(mSpriteMomentum.recentlyMoved()){
		checkBounds();
	}
}

void ScrollArea::handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
	if(ti.mPhase == ds::ui::TouchInfo::Added){
		mSpriteMomentum.deactivate();
	} else if(ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0){
		mSpriteMomentum.activate();
		checkBounds();
	} else if(ti.mPhase == ds::ui::TouchInfo::Moved && ti.mNumberFingers > 0){
		if(mScroller){
			if(mVertical){
				float yDelta = ti.mDeltaPoint.y / ti.mNumberFingers;
				if(getPerspective()){
					yDelta = -yDelta;
				}
				mScroller->move(0.0f, yDelta);
			} else {
				mScroller->move(ti.mDeltaPoint.x / ti.mNumberFingers, 0.0f);
			}
			scrollerUpdated(mScroller->getPosition().xy());
		}
	}

	// notify anyone (like lists) that I was touched so they can prevent action, if they want
	if(mScrollerTouchedFunction){
		mScrollerTouchedFunction();
	}
}

bool ScrollArea::callSnapToPositionCallback(bool& doTween, ci::Vec3f& tweenDestination){
	bool output = false;

	if(mSnapToPositionFunction){
		mSnapToPositionFunction(this, mScroller, doTween, tweenDestination);
		output = true;
	}

	return output;
}

void ScrollArea::setUseFades(const bool doFading){
	if(doFading){
		float fadeWiddy = getWidth();
		float fadeHiddy = getHeight() / 16.0f;
		if(!mVertical){
			fadeWiddy = getWidth() / 16.0f;
			fadeHiddy = getHeight();
		}
		if(!mTopFade){
			mTopFade = new ds::ui::GradientSprite(mEngine);
			if(mTopFade){
				mTopFade->mExportWithXml = false;
				mTopFade->setSize(fadeWiddy, fadeHiddy);
				mTopFade->setTransparent(false);
				mTopFade->enable(false);
				mTopFade->setOpacity(0.0f);
				addChildPtr(mTopFade);
			}
		}

		if(!mBottomFade){
			mBottomFade = new ds::ui::GradientSprite(mEngine);
			if(mBottomFade){
				mBottomFade->mExportWithXml = false;
				mBottomFade->setSize(fadeWiddy, fadeHiddy);
				mBottomFade->setTransparent(false);
				mBottomFade->enable(false);
				mBottomFade->setOpacity(0.0f);
				addChildPtr(mBottomFade);
			}
		}

		setFadeColors(mFadeFullColor, mFadeTransColor);
		setFadeHeight(mFadeHeight);
		setScrollSize(getWidth(), getHeight());
	} else {
		if(mTopFade){
			mTopFade->remove();
			mTopFade = nullptr;
		}

		if(mBottomFade){
			mBottomFade->remove();
			mBottomFade = nullptr;
		}
	}
}

void ScrollArea::setFadeHeight(const float fadeHeight){
	mFadeHeight = fadeHeight;
	float fadeWiddy = getWidth();
	float fadeHiddy = mFadeHeight;
	if(!mVertical){
		fadeWiddy = mFadeHeight;
		fadeHiddy = getHeight();
	}
	if(mTopFade){
		mTopFade->setSize(fadeWiddy, fadeHiddy);
	}

	if(mBottomFade){
		mBottomFade->setSize(fadeWiddy, fadeHiddy);
	}
}

void ScrollArea::setFadeColors(ci::ColorA fadeColorFull, ci::ColorA fadeColorTrans){
	mFadeFullColor = fadeColorFull;
	mFadeTransColor = fadeColorTrans;

	if(mTopFade){
		if(mVertical){
			mTopFade->setColorsAll(fadeColorFull, fadeColorFull, fadeColorTrans, fadeColorTrans);
		} else {
			mTopFade->setColorsAll(fadeColorFull, fadeColorTrans, fadeColorTrans, fadeColorFull);
		}
	}

	if(mBottomFade){
		if(mVertical){
			mBottomFade->setColorsAll(fadeColorTrans, fadeColorTrans, fadeColorFull, fadeColorFull);
		} else {
			mBottomFade->setColorsAll(fadeColorTrans, fadeColorFull, fadeColorFull, fadeColorTrans);
		}
	}
}

void ScrollArea::scrollerUpdated(const ci::Vec2f scrollPos){
	float scrollerSize = mScroller->getHeight();
	float scrollWindow = getHeight();
	float scrollerPossy = scrollPos.y;

	if(!mVertical){
		scrollerSize = mScroller->getWidth();
		scrollWindow = getWidth();
		scrollerPossy = scrollPos.x;
	}
	
	const float theTop = scrollWindow - scrollerSize;

	if(theTop == 0.0f){
		mScrollPercent = 0.0f;
	} else {
		mScrollPercent = scrollerPossy / theTop;
	}
	if(mScrollPercent > 1.0f) mScrollPercent = 1.0f;
	if(mScrollPercent < 0.0f) mScrollPercent = 0.0f;

	if(mTopFade){
		if(scrollerPossy < 0.0f){
			if(!mTopFadeActive){
				mTopFade->tweenOpacity(1.0f, mReturnAnimateTime, 0.0f);
				mTopFadeActive = true;
			}
		} else {
			if(mTopFadeActive){
				mTopFade->tweenOpacity(0.0f, mReturnAnimateTime, 0.0f);
				mTopFadeActive = false;
			}
		}
	}

	if(mBottomFade){
		if(scrollerPossy > theTop){
			if(!mBottomFadeActive){
				mBottomFade->tweenOpacity(1.0f, mReturnAnimateTime, 0.0f);
				mBottomFadeActive = true;
			}
		} else if(mBottomFadeActive){
			mBottomFade->tweenOpacity(0.0f, mReturnAnimateTime, 0.0f);
			mBottomFadeActive = false;
		}
	}

	if(mScrollUpdatedFunction) mScrollUpdatedFunction(this);
}

void ScrollArea::setScrollUpdatedCallback(const std::function<void(ds::ui::ScrollArea* thisThing)> &func){
	mScrollUpdatedFunction = func;
}

void ScrollArea::setTweenCompleteCallback(const std::function<void(ds::ui::ScrollArea* thisThing)> &func){
	mTweenCompleteFunction = func;
}

void ScrollArea::setSnapToPositionCallback(const std::function<void(ScrollArea*, Sprite*, bool&, ci::Vec3f&)>& func){
	mSnapToPositionFunction = func;
}

void ScrollArea::setScrollerTouchedCallback(const std::function<void()>& func) {
	mScrollerTouchedFunction = func;
}

const ci::Vec2f ScrollArea::getScrollerPosition(){
	if(mScroller){
		return mScroller->getPosition().xy();
	}

	return ci::Vec2f::zero();
}

void ScrollArea::resetScrollerPosition() {
	if(mScroller){
		mScroller->animStop();
		if(getPerspective() && mVertical){
			const float theTop = getHeight() - mScroller->getHeight();
			mScroller->setPosition(0.0f, getHeight() - mScroller->getHeight());
		} else {
			mScroller->setPosition(0.0f, 0.0f);
		}
		checkBounds();
	}
}

void ScrollArea::scrollerTweenUpdated(){
	if(mScrollUpdatedFunction){
		mScrollUpdatedFunction(this);
	}
}

void ScrollArea::tweenComplete(){
	if(mTweenCompleteFunction){
		mTweenCompleteFunction(this);
	}
}

float ScrollArea::getScrollPercent(){
	return mScrollPercent;
}

void ScrollArea::setScrollPercent(const float percenty){
	if(!mScroller) return;

	if(mScrollPercent == percenty) return;

	float scrollerSize = mScroller->getHeight();
	float scrollWindow = getHeight();

	if(!mVertical){
		scrollerSize = mScroller->getWidth();
		scrollWindow = getWidth();
	}

	const float theTop = scrollWindow - scrollerSize;

	float scrollerPossy = percenty * theTop;


	if(mVertical){
		if(getPerspective()){
			scrollerPossy = theTop - scrollerPossy;
		}
		mScroller->setPosition(0.0f, scrollerPossy);
	} else {
		mScroller->setPosition(scrollerPossy, 0.0f);
	}
	
	scrollerUpdated(mScroller->getPosition().xy());
}

float ScrollArea::getVisiblePercent(){
	if(!mScroller) return 0.0f;

	if(mVertical){
		if(mScroller->getHeight() < 1.0f) return 1.0f;
		if(mScroller->getHeight() <= getHeight()) return 1.0f;
		
		return getHeight() / mScroller->getHeight();
	} else {
		if(mScroller->getWidth() < 1.0f) return 1.0f;
		if(mScroller->getWidth() <= getWidth()) return 1.0f;

		return getWidth() / mScroller->getWidth();
	}
}

void ScrollArea::scrollPage(const bool forwards, const bool animate) {
	if(!mScroller) return;

	const float visibPerc = getVisiblePercent();
	if(visibPerc == 1.0f) return;


	float scrollerSize = mScroller->getHeight();
	float scrollWindow = getHeight();

	if(!mVertical){
		scrollerSize = mScroller->getWidth();
		scrollWindow = getWidth();
	}

	const float theTop = scrollWindow - scrollerSize;

	float visiblePixes = visibPerc * mScroller->getHeight();
	if(!mVertical)visiblePixes = visibPerc * mScroller->getWidth();

	const float scrolledPerc = getScrollPercent();
	float scrolledPixes = scrolledPerc * theTop;
	if(!mVertical) scrolledPixes = scrolledPerc * theTop;

	float destPixels = scrolledPixes;
	float fadePixels = 0.0f;
	if(mTopFade && mBottomFade){
		if(mVertical){
			fadePixels = mTopFade->getHeight();
		} else {
			fadePixels = mTopFade->getWidth();
		}
	}

	if(forwards){
		destPixels -= (visiblePixes -fadePixels);
	} else {
		destPixels += (visiblePixes - fadePixels);
	}

	if(destPixels < theTop) destPixels = theTop;
	if(destPixels > 0.0f) destPixels = 0.0f;


	ci::Vec3f tweenDestination = mScroller->getPosition();
	if(mVertical){
		if(getPerspective()){
			destPixels = theTop - destPixels;
		}
		tweenDestination.y = destPixels;
	} else {
		tweenDestination.x = destPixels;
	}

	mSpriteMomentum.deactivate();
	if(animate){
		mScroller->tweenPosition(tweenDestination, mReturnAnimateTime, 0.0f, ci::EaseInOutQuint(), nullptr, [this](){ scrollerTweenUpdated(); });
		scrollerUpdated(tweenDestination.xy());
	} else {
		mScroller->animStop();
		mScroller->setPosition(tweenDestination);
		scrollerUpdated(tweenDestination.xy());
	}


}

} // namespace ui

} // namespace ds