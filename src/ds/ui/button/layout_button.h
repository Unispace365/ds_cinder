#pragma once
#ifndef DS_UI_BUTTON_LAYOUT_BUTTON
#define DS_UI_BUTTON_LAYOUT_BUTTON

#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/touch/button_behaviour.h>

namespace ds {
namespace ui {

/**
* \class LayoutButton
*	A convenience class that's basically a SpriteButton that also can Layout like a LayoutSprite.
*	NOTE: changing the layout type of this sprite may make up/down weird
*	This sprite is set to layoutNone, which passes the runLayout() recursive call down, but doesn't affect the size or position of it's children
*	The up/down sprites are set to fillsize and kLayoutSize, so changing the size of this (and running the layout) will make the up/down the same size and affect the size of it's children
*/
class LayoutButton : public ds::ui::LayoutSprite {
public:
	LayoutButton(SpriteEngine& eng, const float widdy = 0.0f, const float hiddy = 0.0f);

	/// the amount of time the images take fading between themselves
	void							setAnimationDuration(const float dur){ mAnimDuration = dur; }

	void							setClickFn(const std::function<void(void)>&);

	/// The visual state has been updated (down or up) pressed = down.
	void							setStateChangeFn(const std::function<void(const bool pressed)>&);

	ds::ui::LayoutSprite&			getNormalSprite();
	ds::ui::LayoutSprite&			getHighSprite();

	void							showDown();
	void							showUp();

	const ButtonBehaviour::State	getButtonState(){ return mButtonBehaviour.getState(); }

private:
	void							onClicked();
	std::function<void(void)>		mClickFn;
	std::function<void(const bool)>	mStateChangeFunction;

	/// VIEW
	ds::ui::LayoutSprite&			mDown;
	ds::ui::LayoutSprite&			mUp;

	/// TOUCH
	ds::ButtonBehaviour				mButtonBehaviour;

	/// SETTINGS
	float							mAnimDuration;


};

} // namespace ui
} // namespace ds

#endif
