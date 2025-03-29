#include "stdafx.h"

#include "layout_button.h"


#include <ds/app/environment.h>

#pragma warning(disable : 4355)

namespace ds { namespace ui {

	/**
	 * \class LayoutButton
	 */
	LayoutButton::LayoutButton(SpriteEngine& eng, float width, float height)
	  : LayoutSprite(eng)
	  , mDown(*(new LayoutSprite(eng)))
	  , mUp(*(new LayoutSprite(eng)))
	  , mButtonBehaviour(*this)
	  , mAnimDuration(0.1f) {

		setLayoutType(kLayoutNone);
		

		setSize(width, height);

		mUp.mExportWithXml	= false;
		mUp.mLayoutUserType = kFillSize;
		mUp.setLayoutType(kLayoutSize);
		addChild(mUp);

		mDown.mExportWithXml  = false;
		mDown.mLayoutUserType = kFillSize;
		mDown.setLayoutType(kLayoutSize);
		mDown.setOpacity(0.0f);
		addChild(mDown);

		mButtonBehaviour.setOnClickFn([this]() { onClicked(); });
		// Purely for visual state
		mButtonBehaviour.setOnDownFn([this](const TouchInfo&) { showDown(); });
		mButtonBehaviour.setOnEnterFn([this]() { showDown(); });
		mButtonBehaviour.setOnExitFn([this]() { showUp(); });
		mButtonBehaviour.setOnUpFn([this]() { showUp(); });
	}

	void LayoutButton::showDown() const {
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

	void LayoutButton::showUp() const {
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

	void LayoutButton::onClicked() const {
		showUp();
		if (mClickFn) mClickFn();
	}


}} // namespace ds::ui
