#include "scroll_bar.h"

#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_list.h>

namespace ds{
namespace ui{

ScrollBar::ScrollBar(ds::ui::SpriteEngine& engine, const bool vertical, const float uiWidth, const float touchPadding, const bool autoHide)
	: Sprite(engine)
	, mVertical(vertical)
	, mNub(nullptr)
	, mBackground(nullptr)
	, mScrollPercent(0.0f)
	, mPercentVisible(1.0f)
	, mMinNubSize(uiWidth)
	, mTouchPadding(touchPadding)
	, mAutoHide(autoHide)
	, mAutoHidden(false)
{

	// Set some defaults
	// You can change these by getting the nub and background sprites and changing them
	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->mExportWithXml = false;
	mBackground->setTransparent(false);
	mBackground->setColor(ci::Color(0.1f, 0.1f, 0.1f));
	mBackground->setCornerRadius(uiWidth/2.0f);
	mBackground->setOpacity(0.7f);
	addChildPtr(mBackground);

	mNub = new ds::ui::Sprite(mEngine);
	mNub->mExportWithXml = false;
	mNub->setTransparent(false);
	mNub->setColor(ci::Color(0.9f, 0.9f, 0.9f));
	mNub->setOpacity(0.3f);
	mNub->setSize(uiWidth, uiWidth);
	mNub->setCornerRadius(uiWidth/2.0f);
	addChildPtr(mNub);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		handleScrollTouch(bs, ti);
	});

	setSize(mTouchPadding * 2.0f + uiWidth, mTouchPadding * 2.0f + uiWidth);

	if(mAutoHide){
		doAutoHide(true);
		hide(); // immediately hide, but also call the function to track the state correctly
		setOpacity(0.0f);
		animOpacityStop();
	}
}


void ScrollBar::handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
	if(ti.mFingerIndex == 0){
		ci::Vec3f localPos = globalToLocal(ti.mCurrentGlobalPoint);
		
		float destPercent;
		if(mVertical){

			// This may not be right. Feel free to fix, but be sure you get it right and check multiple instances
			if(getPerspective()){
				localPos.y -= getHeight()/2.0f;
			}

			destPercent = localPos.y / getHeight();
		} else {
			destPercent = localPos.x / getWidth();
		}

		if(destPercent < 0.0f) destPercent = 0.0f;
		if(destPercent > 1.0f) destPercent = 1.0f;

		if(mScrollMoveCallback){
			mScrollMoveCallback(destPercent);
		}
	}
}


void ScrollBar::setScrollMoveCallback(std::function<void(const float scrollPercent)> func){
	mScrollMoveCallback = func;
}

void ScrollBar::scrollUpdated(const float percentScrolled, const float percentVisible){
	mScrollPercent = percentScrolled;
	if(mScrollPercent < 0.0f) mScrollPercent = 0.0f;
	if(mScrollPercent > 1.0f) mScrollPercent = 1.0f;

	mPercentVisible = percentVisible;
	if(mPercentVisible < 0.0f) mPercentVisible = 0.0f;
	if(mPercentVisible > 1.0f) mPercentVisible = 1.0f;

	updateNubPosition();
}

void ScrollBar::layout(){
	if(mBackground){
		if(mVertical){
			mBackground->setSize(getWidth() - mTouchPadding *2.0f, getHeight());
			mBackground->setPosition(mTouchPadding, 0.0f);
		} else {
			mBackground->setSize(getWidth(), getHeight() - mTouchPadding *2.0f);
			mBackground->setPosition(0.0f, mTouchPadding);
		}
	}

	updateNubPosition();
}

void ScrollBar::onSizeChanged(){
	layout();
}

void ScrollBar::updateNubPosition(){
	if(mNub){
		if(mVertical){
			float nubSize = getHeight() * mPercentVisible;
			if(nubSize < mMinNubSize) {
				nubSize = mMinNubSize;
			}
			mNub->setSize(mNub->getWidth(), nubSize);
			mNub->setPosition(mTouchPadding, mScrollPercent * getHeight() - mScrollPercent * mNub->getHeight());
		} else {
			float nubSize = getWidth() * mPercentVisible;
			if(nubSize < mMinNubSize) {
				nubSize = mMinNubSize;
			}
			mNub->setSize(nubSize, mNub->getHeight());
			mNub->setPosition(getWidth() * mScrollPercent - mScrollPercent * mNub->getWidth(), mTouchPadding);
		}
	}

	if(mAutoHide){
		if(mPercentVisible >= 0.9999f){
			doAutoHide(true);
		} else if(mPercentVisible > 0.000000000000f) {
			doAutoHide(false);
		}
	}

	if(mVisualUpdateCallback){
		mVisualUpdateCallback();
	}
}

void ScrollBar::setVisualUpdateCallback(std::function<void()> func){
	mVisualUpdateCallback = func;
}

ds::ui::Sprite* ScrollBar::getBackgroundSprite(){
	return mBackground;
}

ds::ui::Sprite* ScrollBar::getNubSprite(){
	return mNub;
}

void ScrollBar::setMinNubSize(const float minNub){
	mMinNubSize = minNub; 
}

void ScrollBar::setTouchPadding(const float touchPadding){
	mTouchPadding = touchPadding;
	layout();
}

void ScrollBar::linkScrollArea(ds::ui::ScrollArea* area){
	if(!area) return;

	area->setScrollUpdatedCallback([this](ds::ui::ScrollArea* area){
		scrollUpdated(area->getScrollPercent(), area->getVisiblePercent());

	});
	setScrollMoveCallback([this, area](const float scrollPercent){
		area->setScrollPercent(scrollPercent);
	});
}

void ScrollBar::linkScrollList(ds::ui::ScrollList* list){
	if(!list) return;

	list->setScrollUpdatedCallback([this, list]{
		scrollUpdated(list->getScrollArea()->getScrollPercent(), list->getScrollArea()->getVisiblePercent());
		
	});
	setScrollMoveCallback([this, list](const float scrollPercent){
		if(list->getScrollArea()){
			list->getScrollArea()->setScrollPercent(scrollPercent);
		}
	});
}

void ScrollBar::enableAutoHiding(const bool autoHide){
	mAutoHide = autoHide;
	if(!mAutoHide){
		doAutoHide(false);
	}
}

void ScrollBar::doAutoHide(const bool shouldBeHidden){
	if(shouldBeHidden){
		if(!mAutoHidden){
			mAutoHidden = true;
			tweenOpacity(0.0f, 0.35f, 0.0f, ci::easeNone, [this]{hide(); });
		}
	} else {
		if(mAutoHidden){
			mAutoHidden = false;
			show();
			tweenOpacity(1.0f, 0.35f);
		}
	}

}

} // namespace ui

} // namespace ds