#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUILDER
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUILDER

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "ds/ui/soft_keyboard/soft_keyboard_button.h"
#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

namespace ds {
namespace ui {
	class SoftKeyboard;

	/** 
	*		A convenience for creating soft keyboards in a variety of layouts.
	*		If you want to make a new layout, create a new build function and create Keyboard Buttons as needed
	*/
namespace SoftKeyboardBuilder {

	/// Doesn't include shift, enter, ', \. Good for basic search keyboards
	static SoftKeyboard*			buildLowercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings);

	/// Includes all the regular keys
	static SoftKeyboard*			buildStandardKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings);

	static void						makeAButton(ds::ui::SpriteEngine& engine, SoftKeyboard* parentSprite, float& xPos, float& yPos,
												const std::string& characterLow, const std::string& characterHigh,
												const SoftKeyboardButton::KeyType keyType);

};

} // namespace ui
} // namespace ds

#endif
