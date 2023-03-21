#pragma once
#ifndef DS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUTTON
#define DS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_BUTTON

#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/touch/button_behaviour.h>

#include "ds/ui/soft_keyboard/soft_keyboard_defs.h"
#include "ds/ui/soft_keyboard/soft_keyboard_settings.h"

namespace ds { namespace ui {

	/** SoftKeyboardButton
	 *	A single, tappable key. Holds a couple sprites for text and UI.
	 */
	class SoftKeyboardButton : public ds::ui::Sprite {
	  public:
		SoftKeyboardButton(ds::ui::SpriteEngine&, const std::wstring& characterLower,
						   const std::wstring& characterUpper, const SoftKeyboardDefs::KeyType keyType,
						   SoftKeyboardSettings& softKeySettings);

		/// Sets upper case if true, lower case if false
		void setShifted(const bool upper);

		/// Returns the lower case character; if shifted to upper, returns the upper case character
		const std::wstring& getCharacter();

		/// See the key type above
		const SoftKeyboardDefs::KeyType& getKeyType();

		/// Settings can be changed on-the-fly, induces a layout
		void setSoftKeyboardSettings(SoftKeyboardSettings& softKeySettings);

		/// When the button has been clicked (touch released inside)
		void setClickFn(const std::function<void(void)>&);

	  protected:
		void doLayout();

		void onClicked();
		void showDown();
		void showUp();

		std::function<void(void)>		mClickFn;
		std::function<void(const bool)> mStateChangeFunction;

		float							mAnimDuration;
		ds::ButtonBehaviour				mButtonBehaviour;
		ds::ui::Image*					mDownImg;
		ds::ui::Image*					mUpImg;
		std::wstring					mCharacterLower;
		std::wstring					mCharacterUpper;
		bool							mUpper;
		bool							mPressed;
		const SoftKeyboardDefs::KeyType mKeyType;
		ds::ui::Text*					mText;
		std::string						mTextConfigUp;
		std::string						mTextConfigDown;
		ci::vec2						mTextOffset;

		/// If the keyboard is in graphic mode
		ds::ui::Sprite* mGraphic;
		ci::Color		mKeyUpColor;
		ci::Color		mKeyDnColor;
		double			mLastPressed;
	};
}} // namespace ds::ui

#endif
