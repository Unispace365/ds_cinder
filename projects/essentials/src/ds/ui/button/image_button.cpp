#include "image_button.h"

#include <ds/app/environment.h>

#pragma warning(disable: 4355)

namespace ds {
namespace ui {

/**
* \class ds::ui::ImageButton
*/
ImageButton& ImageButton::makeButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage, const float touchPad, ds::ui::Sprite* parent) {
	ImageButton*		b = new ImageButton(eng, downImage, upImage, touchPad);
	if(!b) throw std::runtime_error("Can't create ImageButton");
	if(parent) parent->addChild(*b);
	return *b;
}

ImageButton::ImageButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage, const float touchPad)
	: inherited(eng)
	, mDown(*(new ds::ui::Image(mEngine, downImage, ds::ui::Image::IMG_CACHE_F)))
	, mUp(*(new ds::ui::Image(mEngine, upImage, ds::ui::Image::IMG_CACHE_F)))
	, mButtonBehaviour(*this)
	, mPad(touchPad)
	, mAnimDuration(0.1f) 
{
// 	setTransparent(false);
// 	setColor(ci::Color(0.5f, 0.8f, 0.2f));

	addChild(mDown);
	addChild(mUp);

	mDown.setOpacity(0.0f);

	mButtonBehaviour.setOnClickFn([this](){onClicked(); });
	// Purely for visual state
	mButtonBehaviour.setOnDownFn([this](const ds::ui::TouchInfo&){showDown(); });
	mButtonBehaviour.setOnEnterFn([this](){showDown(); });
	mButtonBehaviour.setOnExitFn([this](){showUp(); });
	mButtonBehaviour.setOnUpFn([this](){showUp(); });

	layout();
}

void ImageButton::layout(){
	mDown.setPosition(floorf(mPad), floorf(mPad));
	mUp.setPosition(mDown.getPosition());
	setSize(floorf(mPad + mDown.getWidth() + mPad), floorf(mPad + mDown.getHeight() + mPad));
}

void ImageButton::setTouchPad(const float touchPad){
	mPad = touchPad;
	layout();
}

const float ImageButton::getPad() const {
	return mPad;
}

void ImageButton::setClickFn(const std::function<void(void)>& fn) {
	mClickFn = fn;
}

void ImageButton::showDown() {
	if(mAnimDuration <= 0.0f){
		mUp.setOpacity(0.0f);
		mDown.setOpacity(1.0f);
	} else {
		mUp.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
		mDown.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
	}
}

void ImageButton::showUp() {
	if(mAnimDuration <= 0.0f){
		mUp.setOpacity(1.0f);
		mDown.setOpacity(0.0f);
	} else {
		mUp.tweenOpacity(1.0f, mAnimDuration, 0.0f, ci::EaseOutCubic());
		mDown.tweenOpacity(0.0f, mAnimDuration, 0.0f, ci::EaseInCubic());
	}
}

void ImageButton::onClicked() {
	showUp();
	if(mClickFn) mClickFn();

}

ds::ui::Image& ImageButton::getHighImage(){
	return mDown;
}

void ImageButton::setHighImage(const std::string& imageFile){
	mDown.setImageFile(imageFile);
	layout();
}


ds::ui::Image& ImageButton::getNormalImage(){
	return mUp;
}

void ImageButton::setNormalImage(const std::string& imageFile){
	mUp.setImageFile(imageFile);
	layout();
}

} // namespace ui
} // namespace ds

