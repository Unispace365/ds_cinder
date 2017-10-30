#pragma once
#ifndef DS_UI_BUTTON_IMAGE_BUTTON
#define DS_UI_BUTTON_IMAGE_BUTTON

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/touch/button_behaviour.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::::ImageButton
*	A convenience class to create a button from an image or two
*/
class ImageButton : public ds::ui::Sprite {
public:

	static ImageButton&			makeButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage, const float touchPad = 0.0f, ds::ui::Sprite* parent = nullptr);
	ImageButton(SpriteEngine& eng, const std::string& downImage, const std::string& upImage, const float touchPad = 0.0f);

	const float					getPad() const;
	void						setTouchPad(const float touchPad);
	
	// the amount of time the images take fading between themselves
	void						setAnimationDuration(const float dur);

	/// When the button has been clicked (touch released inside)
	void						setClickFn(const std::function<void(void)>&);

	/// The visual state has been updated (down or up) pressed = down.
	void						setStateChangeFn(const std::function<void(const bool pressed)>&);

	ds::ui::Image&				getNormalImage();
	void						setNormalImage(const std::string& imageFile);
	std::string					getNormalImagePath(){ return mNormalFilePath; }

	ds::ui::Image&				getHighImage(); // http://i.imgur.com/1qIw7AV.jpg
	void						setHighImage(const std::string& imageFile);
	std::string					getHighImagePath(){ return mHighFilePath; }

	void						setNormalImageColor(const ci::Color& upColor);
	void						setNormalImageColor(const ci::ColorA& upColor);
	ci::Color					getNormalImageColor(){ return mUp.getColor(); }

	/// Set the color of the image when pressed. Let's you use the same image for both and still have feedback
	void						setHighImageColor(const ci::Color& downColor);
	void						setHighImageColor(const ci::ColorA& downColor);
	ci::Color					getHighImageColor(){ return mDown.getColor(); }

	void						layout();

	void						showDown();
	void						showUp();

	const ButtonBehaviour::State getButtonState(){ return mButtonBehaviour.getState(); }


private:
	void							onClicked();
	std::function<void(void)>		mClickFn;
	std::function<void(const bool)>	mStateChangeFunction;

	// VIEW
	ds::ui::Image&				mDown;
	ds::ui::Image&				mUp;
	std::string					mHighFilePath;
	std::string					mNormalFilePath;

	// TOUCH
	ds::ButtonBehaviour			mButtonBehaviour;

	// SETTINGS
	float						mPad;
	float						mAnimDuration;


};

} // namespace ui
} // namespace ds

#endif