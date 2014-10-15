#include "image_button.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "app/app_defs.h"
#include "app/globals.h"

#pragma warning(disable: 4355)

namespace fullstarter {

/**
 * na::ImageButton2
 */
ImageButton::ImageButton(Globals& g, const std::string& image_filename, const int flags)
		: inherited(g.mEngine)
		, mGlobals(g)
		, mButtonBehaviour(*this)
		, mImage(ds::ui::Sprite::makeAlloc<ds::ui::Image>([&g, &flags]()->ds::ui::Image*{return new ds::ui::Image(g.mEngine, flags); }, this))
		, mImageDown(nullptr)
		, mTouchPadding(0.0f)
		, mState(kStateEnabled)
		, mTouchShrink(g.getSettingsLayout().getFloat("button:touch:pixel_shrink", 0, 0.0f))
		, mAnimDuration(0.1f) {
	setTransparent(true);
	setCenter(0.5f, 0.5f);

	mImage.enable(false);
	mImage.setCenter(0.5f, 0.5f);

	if (!image_filename.empty()) {
		mImage.setImageFile(image_filename);
		layout();
	}

	mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&){onDownFn();});
	mButtonBehaviour.setOnEnterFn([this](){onEnterFn();});
	mButtonBehaviour.setOnExitFn([this](){onExitFn();});
	mButtonBehaviour.setOnUpFn([this](){onUpFn();});
}

void ImageButton::setDownImage(const std::string& filename) {
	if (!mImageDown) {
		mImageDown = new ds::ui::Image(mEngine);
		if (!mImageDown) return;
		addChild(*mImageDown);
		mImageDown->hide();
		mImageDown->setCenter(mImage.getCenter());
		mImageDown->setPosition(mImage.getPosition());
		mImageDown->setScale(mImage.getScale());
	}
	mImageDown->setImageFile(filename);
}

void ImageButton::configure(const std::function<void(ds::ui::Image&)>& fn) {
	if (fn) {
		fn(mImage);
		layout();
	}
}

void ImageButton::setOnClicked(const std::function<void(void)>& fn) {
	if (mImageDown) {
//		mButtonBehaviour.setOnClickFn(fn);
		mButtonBehaviour.setOnClickFn([this, fn]() {
			if (mImageDown) {
				mImage.show();
				mImageDown->hide();
			}
			fn();
		});
		return;	
	}

//	mButtonBehaviour.setOnClickFn(fn);
	// Have a fall-off, so smaller buttons have smaller shrink sizes.
	// Base it on width, 'cause.
	const float		min_w = 30.0f,
					max_w = 90.0f;
	const float		min_amt = 0.5f;
	float			shrink = mTouchShrink;
	const float		w = getWidth();
	if (w <= min_w) {
		shrink *= min_amt;
	} else if (w < max_w) {
		float		amt = (w-min_w)/(max_w - min_w);
		amt = min_amt + (amt * (1.0f - min_amt));
		shrink *= amt;
	}
	mButtonBehaviour.setToScalePixel(shrink, mAnimDuration, fn);
}

void ImageButton::setButtonEnabled(const bool enabled) {
	if (enabled) {
		setOpacity(1.0f);
		enable(true);
	} else {
		setOpacity(0.33f);
		enable(false);
	}
}

void ImageButton::layout() {
	setSize(mImage.getWidth() + (mTouchPadding * 2.0f), mImage.getHeight() + (mTouchPadding * 2.0f));
	mImage.setPosition(floorf(getWidth()/2.0f), floorf(getHeight()/2.0f));
}

void ImageButton::onDownFn() {
	if (!mImageDown) return;

	mImage.hide();
	mImageDown->show();
}

void ImageButton::onEnterFn() {
	if (!mImageDown) return;

	mImage.hide();
	mImageDown->show();
}

void ImageButton::onExitFn() {
	if (!mImageDown) return;

	mImage.show();
	mImageDown->hide();
}

void ImageButton::onUpFn() {
	if (!mImageDown) return;

	mImage.show();
	mImageDown->hide();
}

} // namespace fullstarter
