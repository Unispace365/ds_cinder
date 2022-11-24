#include "stdafx.h"

#include "image_button.h"

#include <ds/app/environment.h>

#pragma warning(disable : 4355)

namespace ds { namespace ui {

	/**
	 * \class ImageButton
	 */
	ImageButton& ImageButton::makeButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage,
										 const float touchPad, ds::ui::Sprite* parent) {
		ImageButton* b = new ImageButton(eng, downImage, upImage, touchPad);
		if (!b) {
			DS_LOG_WARNING("Can't create ImageButton");
			return *b;
		}
		if (parent) parent->addChild(*b);
		return *b;
	}

	ImageButton::ImageButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage,
							 const float touchPad)
	  : ds::ui::Sprite(eng)
	  , mDown(*(new ds::ui::Image(mEngine, downImage, ds::ui::Image::IMG_CACHE_F)))
	  , mUp(*(new ds::ui::Image(mEngine, upImage, ds::ui::Image::IMG_CACHE_F)))
	  , mButtonBehaviour(*this)
	  , mPad(touchPad)
	  , mAnimDuration(0.1f)
	  , mHighFilePath(downImage)
	  , mNormalFilePath(upImage) {
		// 	setTransparent(false);
		// 	setColor(ci::Color(0.5f, 0.8f, 0.2f));

		mLayoutFixedAspect	 = true;
		mDown.mExportWithXml = false;
		mUp.mExportWithXml	 = false;

		addChild(mDown);
		addChild(mUp);

		mDown.setOpacity(0.0f);

		mButtonBehaviour.setOnClickFn([this]() { onClicked(); });
		// Purely for visual state
		mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&) { showDown(); });
		mButtonBehaviour.setOnEnterFn([this]() { showDown(); });
		mButtonBehaviour.setOnExitFn([this]() { showUp(); });
		mButtonBehaviour.setOnUpFn([this]() { showUp(); });

		layout();
	}

	void ImageButton::layout() {
		mDown.setPosition(floorf(mPad), floorf(mPad));
		mUp.setPosition(mDown.getPosition());
		setSize(floorf(mPad + mDown.getWidth() + mPad), floorf(mPad + mDown.getHeight() + mPad));
	}

	void ImageButton::setTouchPad(const float touchPad) {
		mPad = touchPad;
		layout();
	}

	void ImageButton::setAnimationDuration(const float dur) {
		mAnimDuration = dur;
	}

	const float ImageButton::getPad() const {
		return mPad;
	}

	void ImageButton::setClickFn(const std::function<void(void)>& fn) {
		mClickFn = fn;
	}

	void ImageButton::showDown() {
		if (mAnimDuration <= 0.0f) {
			mUp.hide();
			mUp.setOpacity(0.0f);
			mDown.show();
			mDown.setOpacity(1.0f);
		} else {
			mUp.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic(), [this]() { mUp.hide(); });
			mDown.show();
			mDown.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
		}

		if (mStateChangeFunction) {
			mStateChangeFunction(true);
		}
	}

	void ImageButton::showUp() {
		if (mAnimDuration <= 0.0f) {
			mUp.show();
			mUp.setOpacity(1.0f);
			mDown.hide();
			mDown.setOpacity(0.0f);
		} else {
			mUp.show();
			mUp.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
			mDown.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic(), [this]() { mDown.hide(); });
		}

		if (mStateChangeFunction) {
			mStateChangeFunction(false);
		}
	}

	void ImageButton::onClicked() {
		showUp();
		if (mClickFn) mClickFn();
	}

	ds::ui::Image& ImageButton::getHighImage() {
		return mDown;
	}

	void ImageButton::setHighImage(const std::string& imageFile, const int flags) {
		mHighFilePath = imageFile;
		mDown.setImageFile(imageFile, flags);
		layout();
	}


	ds::ui::Image& ImageButton::getNormalImage() {
		return mUp;
	}

	void ImageButton::setNormalImage(const std::string& imageFile, const int flags) {
		if (mNormalFilePath == mHighFilePath) {
			setHighImage(imageFile, flags);
		}
		mNormalFilePath = imageFile;
		mUp.setImageFile(imageFile, flags);
		layout();
	}

	void ImageButton::setStateChangeFn(const std::function<void(const bool pressed)>& func) {
		mStateChangeFunction = func;
	}

	void ImageButton::setNormalImageColor(const ci::Color& upColor) {
		mUp.setColor(upColor);
	}

	void ImageButton::setNormalImageColor(const ci::ColorA& upColor) {
		mUp.setColorA(upColor);
	}

	void ImageButton::setHighImageColor(const ci::Color& downColor) {
		mDown.setColor(downColor);
	}

	void ImageButton::setHighImageColor(const ci::ColorA& downColor) {
		mDown.setColorA(downColor);
		showUp();
	}

}} // namespace ds::ui
