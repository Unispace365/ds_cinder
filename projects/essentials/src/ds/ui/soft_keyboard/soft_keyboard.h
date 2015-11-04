#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD

#include <ds/ui/sprite/sprite.h>

#include "ds/ui/soft_keyboard/soft_keyboard_button.h"
#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

namespace ds {
namespace ui {

	/**  Soft Keyboard
	*		Owns the keyboard buttons, manages touch functions, and tracks the entered text (adds on most keys, subtracts on delete)	
	*/
class SoftKeyboard : public ds::ui::Sprite {
public:

	SoftKeyboard(ds::ui::SpriteEngine&, SoftKeyboardSettings& settings);

	/// Get the current entered text string
	const std::wstring&					getCurrentText();

	/// Set the the current text string. Does not call any callbacks.
	void								setCurrentText(const std::wstring& curTxtStr);

	/// Clears the current text string. Does not call any callbacks.
	void								resetCurrentText();

	/// Makes text uppercase or lowercase on a toggle
	void								toggleShift();

	/// Set a callback for every time a key is pressed by the user. KeyType is set in SoftKeyboardButton (letter, number, back, space, delete, etc)
	void								setKeyPressFunction(const std::function<void(const std::wstring& character, const SoftKeyboardDefs::KeyType keyType)>&);

	/// Provided for class that create keyboard buttons externally (such as SoftKeyboardBuilder). 
	void								handleKeyPress(SoftKeyboardButton* sp);

	/// Returns a reference to the vector of all the Keyboard Buttons
	std::vector<SoftKeyboardButton*>&	getButtonVector(){ return mButtons; }

	/// Returns a reference to the settings for the keyboard
	SoftKeyboardSettings&				getSoftKeyboardSettings(){ return mSoftKeyboardSettings; }

protected:
	std::wstring						mCurrentText;
	bool								mUpperCase;

	std::function<void(const std::wstring& character, const SoftKeyboardDefs::KeyType keyType)> mKeyPressFunction;
	std::vector<SoftKeyboardButton*>	mButtons;
	SoftKeyboardSettings				mSoftKeyboardSettings;

};

} // namespace ui
} // namespace ds

#endif
