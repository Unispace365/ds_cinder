#include "control_slider.h"

#include <ds/math/math_func.h>

namespace ds{
namespace ui{

ControlSlider::ControlSlider(ds::ui::SpriteEngine& engine, const bool vertical, const float uiSize, const float slideLength)
	: Sprite(engine)
	, mVertical(vertical)
	, mNub(nullptr)
	, mBackground(nullptr)
	, mSliderPercent(0.0)
	, mMinValue(0.0)
	, mMaxValue(1.0)
	, mTouchPadding(20.0f)
	, mSliderInterpolation(kSliderTypeLinear)
{

	// Set some defaults
	// You can change these by getting the nub and background sprites and changing them
	mBackground = new ds::ui::Sprite(mEngine);
	mBackground->mExportWithXml = false;
	mBackground->setTransparent(false);
	mBackground->setColor(ci::Color(0.1f, 0.1f, 0.1f));
	mBackground->setCornerRadius(uiSize / 2.0f);
	addChildPtr(mBackground);

	mNub = new ds::ui::Sprite(mEngine);
	mNub->mExportWithXml = false;
	mNub->setTransparent(false);
	mNub->setColor(ci::Color(0.4f, 0.4f, 0.4f));
	mNub->setCornerRadius(uiSize / 2.0f);
	addChildPtr(mNub);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		handleScrollTouch(bs, ti);
	});

	if(mVertical){
		mNub->setSize(uiSize * 4.0f, uiSize);
		setSize(mTouchPadding * 2.0f + uiSize, mTouchPadding * 2.0f + slideLength);
	} else {
		mNub->setSize(uiSize, uiSize * 4.0f);
		setSize(mTouchPadding * 2.0f + slideLength, mTouchPadding * 2.0f + uiSize);
	}
}

void ControlSlider::handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
	if(ti.mFingerIndex == 0){
		ci::Vec3f localPos = globalToLocal(ti.mCurrentGlobalPoint);

		bool isPerspective = getPerspective();
		float heighty = getHeight();
		if(!mVertical){
			heighty = getWidth();
		}

		float totalHeight = heighty;

		double destPercent = 0.0;

		if(mVertical){
			// This may not be right. Feel free to fix, but be sure you get it right and check multiple instances
			if(isPerspective){
				localPos.y -= getHeight() / 2.0f;
			}

			destPercent = (double)(localPos.y / totalHeight);

			if(isPerspective){
				destPercent = 1.0 - destPercent;
			}

		} else {
			//totalHeight = getWidth() - getWidth();
			destPercent = localPos.x / totalHeight;
		}

		if(destPercent < 0.0) destPercent = 0.0;
		if(destPercent > 1.0) destPercent = 1.0;

		if(mSliderUpdatedCallback){
			double sliderValue = 0.0;
			if(mSliderInterpolation == kSliderTypeLinear){
				sliderValue = ds::math::convertRange(0.0, 1.0, mMinValue, mMaxValue, destPercent);
			} else if(mSliderInterpolation == kSliderTypeQuadratic){
				// y = x ^ 2
				double val = destPercent * destPercent;
				sliderValue = ds::math::convertRange(0.0, 1.0, mMinValue, mMaxValue, val);
			}
			mSliderUpdatedCallback(destPercent, sliderValue, ti.mPhase == ds::ui::TouchInfo::Removed);
		}

		sliderUpdated(destPercent);
	}
}


void ControlSlider::setSliderUpdatedCallback(std::function<void(const double scrollPercent, const double sliderValue, const bool finishedAdjusting)> func){
	mSliderUpdatedCallback = func;
}

void ControlSlider::sliderUpdated(const double sliderPercent){
	mSliderPercent = sliderPercent;
	if(mSliderPercent < 0.0) mSliderPercent = 0.0;
	if(mSliderPercent > 1.0) mSliderPercent = 1.0;

	updateNubPosition();
}

void ControlSlider::setSliderValue(const double sliderValue){
	if(mSliderInterpolation == kSliderTypeLinear){
		mSliderPercent = ds::math::convertRange(mMinValue, mMaxValue, 0.0, 1.0, sliderValue);
	} else if(mSliderInterpolation == kSliderTypeQuadratic){
		mSliderPercent = ds::math::convertRange(mMinValue, mMaxValue, 0.0, 1.0, sliderValue);
		mSliderPercent = sqrt(mSliderPercent);
	}
	if(mSliderPercent < 0.0) mSliderPercent = 0.0;
	if(mSliderPercent > 1.0) mSliderPercent = 1.0;

	updateNubPosition();
}

void ControlSlider::layout(){
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

void ControlSlider::onSizeChanged(){
	layout();
}

void ControlSlider::updateNubPosition(){
	if(mNub){
		if(mVertical){

			//mNub->setSize(getWidth(), mNub->getHeight());
			
			if(getPerspective()){
				mNub->setPosition(mTouchPadding, (1.0f - (float)mSliderPercent) * (getHeight() - mNub->getHeight()));
			} else {
				mNub->setPosition(mTouchPadding, (float)mSliderPercent * getHeight() - (float)mSliderPercent * mNub->getHeight());
			}
		} else {
			
		//	mNub->setSize(mNub->getWidth(), getHeight());
			
			mNub->setPosition(getWidth() * (float)mSliderPercent - (float)mSliderPercent * mNub->getWidth(), getHeight() / 2.0f - mNub->getHeight() / 2.0f);
		}
	}

	if(mVisualUpdateCallback){
		mVisualUpdateCallback();
	}
}

void ControlSlider::setVisualUpdateCallback(std::function<void()> func){
	mVisualUpdateCallback = func;
}

ds::ui::Sprite* ControlSlider::getBackgroundSprite(){
	return mBackground;
}

ds::ui::Sprite* ControlSlider::getNubSprite(){
	return mNub;
}

void ControlSlider::setTouchPadding(const float touchPadding){
	mTouchPadding = touchPadding;
	layout();
}

void ControlSlider::setSliderLimits(const double minValue, const double maxValue){
	mMinValue = minValue;
	mMaxValue = maxValue;
}

} // namespace ui

} // namespace ds