#include "stdafx.h"

#include "sprite_button.h"

#include <ds/app/environment.h>

#pragma warning(disable: 4355)

namespace ds {
namespace ui {

/**
* \class SpriteButton
*/
SpriteButton::SpriteButton(SpriteEngine& eng, const float widdy, const float hiddy)
	: inherited(eng, widdy, hiddy)
	, mDown(ds::ui::Sprite::makeSprite(eng, this))
	, mUp(ds::ui::Sprite::makeSprite(eng, this))
	, mButtonBehaviour(*this)
	, mAnimDuration(0.1f)
{

	mUp.mExportWithXml = false;
	mDown.mExportWithXml = false;

	mDown.setOpacity(0.0f);

	mButtonBehaviour.setOnClickFn([this](){onClicked(); });
	// Purely for visual state
	mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&){showDown(); });
	mButtonBehaviour.setOnEnterFn([this](){showDown(); });
	mButtonBehaviour.setOnExitFn([this](){showUp(); });
	mButtonBehaviour.setOnUpFn([this](){showUp(); });

}

void SpriteButton::setClickFn(const std::function<void(void)>& fn) {
	mClickFn = fn;
}

void SpriteButton::showDown() {
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

void SpriteButton::showUp() {
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

YGSize SpriteButton::yogaMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode)
{
	YGSize retVal;
	ds::ui::SpriteButton* spr = this;
	auto bb_h = spr->getHighSprite().getChildBoundingBox();
	auto bb_n = spr->getNormalSprite().getChildBoundingBox();
	bb_h.include(bb_n);
	spr->setSize(bb_h.getSize());
	retVal.width = spr->getWidth();
	retVal.height = spr->getHeight();
	return retVal;
}

void SpriteButton::onClicked() {
	showUp();
	if(mClickFn) mClickFn();
}

ds::ui::Sprite& SpriteButton::getHighSprite(){
	return mDown;
}

ds::ui::Sprite& SpriteButton::getNormalSprite(){
	return mUp;
}

void SpriteButton::setStateChangeFn(const std::function<void(const bool pressed)>& func) {
	mStateChangeFunction = func;
}


} // namespace ui
} // namespace ds

