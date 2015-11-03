#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUTTON
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUTTON

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/text.h>

#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

namespace ds {
namespace ui {

	/** SoftKeyboardButton
	*	A single, tappable key. Holds a couple sprites for text and UI.
	*/
class SoftKeyboardButton : public ds::ui::ImageButton {
public:

	typedef enum { kLetter, kNumber, kDelete, kSpace, kEnter, kShift } KeyType;

	SoftKeyboardButton(ds::ui::SpriteEngine&, const std::string& characterLower, const std::string& characterUpper, const KeyType keyType, SoftKeyboardSettings& softKeySettings);

	void						setToggle(const bool upper);
	std::string&				getCharacter();
	const KeyType&				getKeyType();

	void						setSoftKeyboardSettings(SoftKeyboardSettings& softKeySettings);

protected:
	void						stateChanged(const bool pressed);
	void						layout();

	std::string					mCharacterLower;
	std::string					mCharacterUpper;
	bool						mUpper;
	bool						mPressed;
	const KeyType				mKeyType;
	ds::ui::Text*				mText;
	std::string					mTextConfigUp;
	std::string					mTextConfigDown;
	ci::Vec2f					mTextOffset;

};
} // namespace ui
} // namespace ds

#endif
