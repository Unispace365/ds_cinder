#include "scroll_area.h"


namespace ds{
namespace ui{

ScrollArea::ScrollArea(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool vertical)
	: Sprite(engine)
	, mSpriteMomentum(engine)
	, mScroller(nullptr)
	, mScrollable(false)
	, mReturnAnimateTime(0.3f)
	, mTopFade(nullptr)
	, mBottomFade(nullptr)
	, mTopFadeActive(false)
	, mBottomFadeActive(false)
	, mFadeHeight(30.0f)
	, mFadeFullColor(0, 0, 0, 255)
	, mFadeTransColor(0, 0, 0, 0)
	, mScrollUpdatedFunction(nullptr)
	, mVertical(vertical)
{

	setSize(startWidth, startHeight);
	mSpriteMomentum.setMass(8.0f);
	mSpriteMomentum.setFriction(0.5f);
	mSpriteMomentum.setMomentumParent(this);

	enable(false);
	setClipping(true);

	mScroller = new Sprite(mEngine);
	if(mScroller){
#if 0
		mScroller->setTransparent(false);
		mScroller->setColor(ci::Color(0.1f, 0.56f, 0.3f));
#endif
		mScroller->setSize(startWidth, startHeight);
		mScroller->enable(true);
		mScroller->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mScroller->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti){handleScrollTouch(bs, ti); });
		mSpriteMomentum.setMomentumParent(mScroller);
		addChildPtr(mScroller);
	}
}

void ScrollArea::setScrollSize(const float newWidth, const float newHeight){
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
	setSize(newWidth, newHeight);
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
	float scrollWindow(0.0f);
	float scrollerSize(0.0f);

	if(mVertical){
		scrollWindow = getHeight();
		scrollerSize = mScroller->getHeight();
	} else {
		scrollWindow = getWidth();
		scrollerSize = mScroller->getWidth();
	}

	if(scrollerSize <= scrollWindow){
		mScrollable = false;
		mSpriteMomentum.deactivate();
		mScroller->tweenPosition(ci::Vec3f::zero(), mReturnAnimateTime, 0.0f, ci::EaseOutQuint(), nullptr, [this](){ scrollerTweenUpdated(); });
		scrollerUpdated(ci::Vec2f(0.0f, 0.0f));
	} else {
		bool isPerspective = getPerspective();
		float scrollerPos(0.0f);
		float theTop = scrollWindow - scrollerSize;
		if(mVertical){
			scrollerPos = mScroller->getPosition().y;
		} else {
			scrollerPos = mScroller->getPosition().x;
		}

		bool doTween = true;
		ci::Vec3f tweenDestination = ci::Vec3f::zero();

		// Perspective y-position works in opposite
		if(isPerspective && mVertical){
			if(scrollerPos > 0){
				doTween = true;
				tweenDestination = ci::Vec3f::zero();

			} else if(scrollerPos < theTop){
				doTween = true;
				tweenDestination = ci::Vec3f(0.0f, theTop, 0.0f);
			} else {
				doTween = false;
			}

		} else {

			// Can't scroll down any more
			if(scrollerPos > 0){
				doTween = true;
				tweenDestination = ci::Vec3f::zero();

			// Can't scroll up any more
			} else if(scrollerPos < theTop){
				doTween = true;
				if(mVertical){
					tweenDestination = ci::Vec3f(0.0f, theTop, 0.0f);
				} else {
					tweenDestination = ci::Vec3f(theTop, 0.0f, 0.0f);
				}

			// In bounds
			} else {
				doTween = false;
			}
		}


		if(doTween){
			mSpriteMomentum.deactivate();
			mScroller->tweenPosition(tweenDestination, mReturnAnimateTime, 0.0f, ci::EaseOutQuint(), nullptr, [this](){ scrollerTweenUpdated(); });
			scrollerUpdated(tweenDestination.xy());
		} else {
			scrollerUpdated(mScroller->getPosition().xy());
		}
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
	if(!mTopFade || !mBottomFade) return;

	float scrollerSize = mScroller->getHeight();
	float scrollWindow = getHeight();
	float scrollerPossy = scrollPos.y;

	if(!mVertical){
		scrollerSize = mScroller->getWidth();
		scrollWindow = getWidth();
		scrollerPossy = scrollPos.x;
	}

	const float theTop = scrollWindow - scrollerSize;

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

	if(scrollerPossy > theTop){
		if(!mBottomFadeActive){
			mBottomFade->tweenOpacity(1.0f, mReturnAnimateTime, 0.0f);
			mBottomFadeActive = true;
		}
	} else if(mBottomFadeActive){
		mBottomFade->tweenOpacity(0.0f, mReturnAnimateTime, 0.0f);
		mBottomFadeActive = false;
	}

	if(mScrollUpdatedFunction) mScrollUpdatedFunction(this);
}

void ScrollArea::setScrollUpdatedCallback(const std::function<void(ds::ui::ScrollArea* thisThing)> &func){
	mScrollUpdatedFunction = func;
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
	if(mScrollUpdatedFunction) mScrollUpdatedFunction(this);
}

} // namespace ui

} // namespace ds