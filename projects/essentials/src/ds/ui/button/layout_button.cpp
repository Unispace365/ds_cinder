#include "stdafx.h"

#include "layout_button.h"

#include <ds/app/environment.h>

#pragma warning(disable: 4355)

namespace ds {
namespace ui {

/**
* \class ds::ui::LayoutButton
*/
LayoutButton::LayoutButton(SpriteEngine& eng, const float widdy, const float hiddy)
	: ds::ui::LayoutSprite(eng)
	, mDown(*(new ds::ui::LayoutSprite(eng)))
	, mUp(*(new ds::ui::LayoutSprite(eng)))
	, mButtonBehaviour(*this)
	, mAnimDuration(0.1f)
{

	setLayoutType(ds::ui::LayoutSprite::kLayoutNone);

	mUp.mExportWithXml = false;
	mUp.mLayoutUserType = ds::ui::LayoutSprite::kFillSize;
	mUp.setLayoutType(ds::ui::LayoutSprite::kLayoutSize);
	addChild(mUp);

	mDown.mExportWithXml = false;
	mDown.mLayoutUserType = ds::ui::LayoutSprite::kFillSize;
	mDown.setLayoutType(ds::ui::LayoutSprite::kLayoutSize);
	mDown.setOpacity(0.0f);
	addChild(mDown);

	mButtonBehaviour.setOnClickFn([this](){onClicked(); });
	// Purely for visual state
	mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&){showDown(); });
	mButtonBehaviour.setOnEnterFn([this](){showDown(); });
	mButtonBehaviour.setOnExitFn([this](){showUp(); });
	mButtonBehaviour.setOnUpFn([this](){showUp(); });

}

void LayoutButton::setClickFn(const std::function<void(void)>& fn) {
	mClickFn = fn;
}

void LayoutButton::showDown() {
	if(mAnimDuration <= 0.0f){
		mUp.setOpacity(0.0f);
		mDown.setOpacity(1.0f);
	} else {
		mUp.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
		mDown.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
	}

	if(mStateChangeFunction){
		mStateChangeFunction(true);
	}
}

void LayoutButton::showUp() {
	if(mAnimDuration <= 0.0f){
		mUp.setOpacity(1.0f);
		mDown.setOpacity(0.0f);
	} else {
		mUp.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
		mDown.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
	}

	if(mStateChangeFunction){
		mStateChangeFunction(false);
	}
}

void LayoutButton::onClicked() {
	showUp();
	if(mClickFn) mClickFn();
}

ds::ui::LayoutSprite& LayoutButton::getHighSprite(){
	return mDown;
}

ds::ui::LayoutSprite& LayoutButton::getNormalSprite(){
	return mUp;
}

void LayoutButton::setStateChangeFn(const std::function<void(const bool pressed)>& func) {
	mStateChangeFunction = func;
}


} // namespace ui
} // namespace ds

