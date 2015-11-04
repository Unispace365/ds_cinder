#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_ENTRY_FIELD
#define ESSENTIALS_UI_SOFT_KEYBOARD_ENTRY_FIELD

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/multiline_text.h>

#include "ds/ui/soft_keyboard/soft_keyboard_defs.h"

namespace ds {
namespace ui {

/**  EntryField
*		A text field with a blinking cursor for text entry.
*/

class EntryFieldSettings {
public:
	EntryFieldSettings()
		: mTextConfig("entry_field:text")
		, mCursorSize(2.0f, 36.0f)
		, mCursorPadding(2.0f)
		, mCursorColor(1.0f, 1.0f, 1.0f)
		, mBlinkRate(0.5f)
		, mAnimationRate(0.3f)
	{}

	std::string mTextConfig;
	ci::Vec2f	mCursorSize;
	ci::Color	mCursorColor;
	float		mCursorPadding; // the space between the current piece of text and the cursor
	float		mBlinkRate;
	float		mAnimationRate;
};

class EntryField : public ds::ui::Sprite {
public:

	EntryField(ds::ui::SpriteEngine&, EntryFieldSettings& settings);

	/// This field will be in "focus", making the cursor blink and look like this is activated
	/// Does no checking about the number of fields in focus, and any number could be in focus at once (multiple user scenario)
	void								focus();

	/// This field will lose focus, which hides the cursor, and this field won't be active
	void								unfocus();

	/// Get the current entered text string
	const std::wstring					getCurrentText();

	/// Set the the current text string. Does not call any callbacks.
	void								setCurrentText(const std::wstring& curTxtStr);

	/// Clears the current text string. Does not call any callbacks.
	void								resetCurrentText();

	/// Settings can be set at any time to update graphic properties
	void								setEntryFieldSettings(EntryFieldSettings& newSettings);

	/// Handles key input like a keyboard would
	void								keyPressed(const std::wstring& keyCharacter, const ds::ui::SoftKeyboardDefs::KeyType keyType);

	/// If this sprite's text can be selected through touch input
	void								setSelectable(const bool isSelectable);

protected:

	/// Override to know when this field gains focus.
	virtual void						onFocus(){}

	/// Override to know when this field loses focus
	virtual void						onUnFocus(){}

	/// Any time the text has changed, do not override
	void								textUpdated();

	/// Override this to know when any text has changed
	virtual void						onTextUpdated(){}

	/// A series of tweens that fades the cursor on, waits, then fades it off
	void								blinkCursor();

	bool								mInFocus;
	int									mCursorIndex;

	ds::ui::MultilineText*				mTextSprite;
	ds::ui::Sprite*						mCursor;
	std::vector<ds::ui::Sprite*>		mSelectionIndicators;
	EntryFieldSettings					mEntryFieldSettings;

};

} // namespace ui
} // namespace ds

#endif
