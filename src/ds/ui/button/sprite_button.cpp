#include "stdafx.h"

#include "sprite_button.h"

#include <ds/app/environment.h>

#pragma warning(disable : 4355)

namespace ds { namespace ui {

	/**
	 * \class SpriteButton
	 */
	SpriteButton::SpriteButton(SpriteEngine& eng, float width, float height)
	  : Sprite(eng, width, height)
	  , mDown(*(new AutoSizeSprite(eng)))
	  , mUp(*(new AutoSizeSprite(eng)))
	  , mButtonBehaviour(*this)
	  , mPad(0)
	  , mAnimDuration(0.1f) {

		addChild(mUp);
		addChild(mDown);

		mUp.setDimensionsChangedCallback([this](Sprite*) { handleResize(); });
		mDown.setDimensionsChangedCallback([this](Sprite*) { handleResize(); });

		mUp.mExportWithXml	 = false;
		mDown.mExportWithXml = false;

		mDown.setOpacity(0.0f);

		mButtonBehaviour.setOnClickFn([this]() { onClicked(); });
		// Purely for visual state
		mButtonBehaviour.setOnDownFn([this](const TouchInfo&) { showDown(); });
		mButtonBehaviour.setOnEnterFn([this]() { showDown(); });
		mButtonBehaviour.setOnExitFn([this]() { showUp(); });
		mButtonBehaviour.setOnUpFn([this]() { showUp(); });
	}

	void SpriteButton::setTouchPad(float touchPad) {
		mPad = touchPad;
		handleResize();
	}

	void SpriteButton::showDown() const {
		if (mAnimDuration <= 0.0f) {
			mUp.setOpacity(0.0f);
			mDown.setOpacity(1.0f);
		} else {
			mUp.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
			mDown.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
		}

		if (mStateChangeFunction) {
			mStateChangeFunction(true);
		}
	}

	void SpriteButton::showUp() const {
		if (mAnimDuration <= 0.0f) {
			mUp.setOpacity(1.0f);
			mDown.setOpacity(0.0f);
		} else {
			mUp.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
			mDown.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
		}

		if (mStateChangeFunction) {
			mStateChangeFunction(false);
		}
	}

	void SpriteButton::handleResize() {
		mDown.setPosition(mPad, mPad);
		mUp.setPosition(mDown.getPosition());

		auto bounds = mDown.getChildBoundingBox();
		bounds.include(mUp.getChildBoundingBox());
		setSize(mPad + bounds.getWidth() + mPad, mPad + bounds.getHeight() + mPad);
	}

	YGSize SpriteButton::yogaMeasureFunc(YGNodeRef node, float width, YGMeasureMode widthMode, float height,
										 YGMeasureMode heightMode) {
		YGSize retVal;
		retVal.width  = getWidth();
		retVal.height = getHeight();
		return retVal;
	}

	void SpriteButton::onClicked() const {
		showUp();
		if (mClickFn) mClickFn();
	}


}} // namespace ds::ui
