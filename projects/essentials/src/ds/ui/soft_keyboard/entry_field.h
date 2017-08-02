#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_ENTRY_FIELD
#define ESSENTIALS_UI_SOFT_KEYBOARD_ENTRY_FIELD

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

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
		, mFieldSize(500.0f, 100.0f)
		, mCursorOffset(2.0f, -5.0f)
		, mCursorColor(1.0f, 1.0f, 1.0f)
		, mBlinkRate(0.5f)
		, mAnimationRate(0.3f)
		, mPasswordMode(false)
		, mTextOffset(0.0f, 0.0f)
	{}

	std::string mTextConfig;
	ci::vec2	mTextOffset;
	ci::vec2	mFieldSize;
	ci::vec2	mCursorSize;
	ci::Color	mCursorColor;
	ci::vec2	mCursorOffset; 
	bool		mPasswordMode;
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

	/// Set a lambda function when the above keypressed function is called
	void								setKeyPressedCallback(std::function<void(const std::wstring& keyCharacter, const ds::ui::SoftKeyboardDefs::KeyType keyType)> keyPressedFunc);

	/// Set a lambda function that's called just after onTextUpdated()
	void								setTextUpdatedCallback(std::function<void(const std::wstring& fullStr)> func);

protected:

	/// Override to know when this field gains focus.
	virtual void						onFocus(){}

	/// Override to know when this field loses focus
	virtual void						onUnFocus(){}

	virtual void						onSizeChanged() override;

	/// Any time the text has changed, do not override
	void								textUpdated();

	/// The cursor position has been changed, do not override. Also called on text updated
	void								cursorUpdated();

	/// Override this to know when any text has changed
	virtual void						onTextUpdated(){}

	/// A series of tweens that fades the cursor on, waits, then fades it off
	void								blinkCursor();

	void								handleTouchInput(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);

	/// Sets the text sprite's text (bullets for pswd, normal text for normal text)
	void								applyText(const std::wstring& theStr);

	bool								mInFocus;
	size_t								mCursorIndex;

	std::wstring						mCurrentText;
	ds::ui::Text*						mTextSprite;
	ds::ui::Sprite*						mCursor;
	std::vector<ds::ui::Sprite*>		mSelectionIndicators;
	EntryFieldSettings					mEntryFieldSettings;
	std::function<void(const std::wstring&, const ds::ui::SoftKeyboardDefs::KeyType)> mKeyPressedFunction;
	std::function<void(const std::wstring&)> mTextUpdateFunction;

};

} // namespace ui
} // namespace ds

#endif
