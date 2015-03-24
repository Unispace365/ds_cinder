#pragma once
#ifndef DS_UI_BUTTON_SPRITE_BUTTON
#define DS_UI_BUTTON_SPRITE_BUTTON

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/touch/button_behaviour.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::::SpriteButton
*	A convenience class to create a button for some basic sprites
*/
class SpriteButton : public ds::ui::Sprite {
public:
	SpriteButton(SpriteEngine& eng, const float widdy = 0.0f, const float hiddy = 0.0f);

	// the amount of time the images take fading between themselves
	void						setAnimationDuration(const float dur);

	void						setClickFn(const std::function<void(void)>&);

	ds::ui::Sprite&				getNormalSprite();
	ds::ui::Sprite&				getHighSprite(); // http://i.imgur.com/1qIw7AV.jpg

	void						showDown();
	void						showUp();

	const ButtonBehaviour::State getButtonState(){ return mButtonBehaviour.getState(); }

private:
	void						onClicked();
	typedef ds::ui::Sprite		inherited;
	std::function<void(void)>	mClickFn;

	// VIEW
	ds::ui::Sprite&				mDown;
	ds::ui::Sprite&				mUp;

	// TOUCH
	ds::ButtonBehaviour			mButtonBehaviour;

	// SETTINGS
	const float					mAnimDuration;


};

} // namespace ui
} // namespace ds

#endif