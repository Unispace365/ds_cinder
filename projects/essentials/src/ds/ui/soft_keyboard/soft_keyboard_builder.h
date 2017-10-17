#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUILDER
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUILDER

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "ds/ui/soft_keyboard/soft_keyboard_defs.h"
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
	SoftKeyboard*			buildLowercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);
	SoftKeyboard*			buildUppercaseKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr, bool numbers = false);

	/// Includes all the regular keys
	SoftKeyboard*			buildStandardKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);

	/// Includes keys required for email and web input
	SoftKeyboard*			buildExtendedKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);

	/// Includes function keys, home/end, page up/dn, arrow keys
	SoftKeyboard*			buildFullKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);

	/// Number entry with an "enter" button at the bottom
	SoftKeyboard*			buildPinPadKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);
	/// Number entry with a "delete" key at the bottom
	SoftKeyboard*			buildPinCodeKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings, ds::ui::Sprite* parent = nullptr);

	SoftKeyboardButton*		makeAButton(ds::ui::SpriteEngine& engine, SoftKeyboard* parentSprite, float& xPos, float& yPos,
												const std::wstring& characterLow, const std::wstring& characterHigh,
												const SoftKeyboardDefs::KeyType keyType);

};

} // namespace ui
} // namespace ds

#endif
