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

	void						setClickFn(const std::function<void(void)>&);

	ds::ui::Image&				getNormalImage();
	void						setNormalImage(const std::string& imageFile);
	ds::ui::Image&				getHighImage(); // http://i.imgur.com/1qIw7AV.jpg
	void						setHighImage(const std::string& imageFile);

	void						layout();

	void						showDown();
	void						showUp();

	const ButtonBehaviour::State getButtonState(){ return mButtonBehaviour.getState(); }

private:
	void						onClicked();
	typedef ds::ui::Sprite		inherited;
	std::function<void(void)>	mClickFn;

	// VIEW
	ds::ui::Image&				mDown;
	ds::ui::Image&				mUp;

	// TOUCH
	ds::ButtonBehaviour			mButtonBehaviour;

	// SETTINGS
	float						mPad;
	const float					mAnimDuration;


};

} // namespace ui
} // namespace ds

#endif