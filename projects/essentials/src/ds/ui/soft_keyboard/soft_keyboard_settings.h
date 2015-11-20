#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "ds/ui/soft_keyboard/soft_keyboard_button.h"

namespace ds {
namespace ui {
class SoftKeyboardSettings {

public:
	SoftKeyboardSettings()
		: mKeyTouchPadding(4.0f)
		, mKeyInitialPosition(0.0f, 0.0f)
		, mKeyTextOffset(-5.0f, -5.0f)
		, mKeyDownColor(0.5f, 0.5f, 0.5f)
		, mKeyLetterDnImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyLetterUpImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyNumberDnImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyNumberUpImage("%APP%/data/images/keyboard/Normal.png")
		, mKeySpaceDnImage("%APP%/data/images/keyboard/Space.png")
		, mKeySpaceUpImage("%APP%/data/images/keyboard/Space.png")
		, mKeyDeleteDnImage("%APP%/data/images/keyboard/Delete.png")
		, mKeyDeleteUpImage("%APP%/data/images/keyboard/Delete.png")
		, mKeyEnterDnImage("%APP%/data/images/keyboard/EnterAndShift.png")
		, mKeyEnterUpImage("%APP%/data/images/keyboard/EnterAndShift.png")
		, mKeyShiftDnImage("%APP%/data/images/keyboard/EnterAndShift.png")
		, mKeyShiftUpImage("%APP%/data/images/keyboard/EnterAndShift.png")
		, mKeyUpTextConfig("keyboard:key:up")
		, mKeyDnTextConfig("keyboard:key:down")
	{}

	void				normalizeSettings(){
		if(mKeyLetterDnImage.empty()){
			mKeyLetterDnImage = mKeyLetterUpImage;
		}
		if(mKeyNumberUpImage.empty()){
			mKeyNumberUpImage = mKeyLetterUpImage;
		}
		if(mKeyNumberDnImage.empty()){
			mKeyNumberDnImage = mKeyNumberUpImage;
		}
		if(mKeySpaceDnImage.empty()){
			mKeySpaceDnImage = mKeySpaceUpImage;
		}
		if(mKeyDeleteDnImage.empty()){
			mKeyDeleteDnImage = mKeyDeleteUpImage;
		}
		if(mKeyEnterDnImage.empty()){
			mKeyEnterDnImage = mKeyEnterUpImage;
		}
		if(mKeyShiftDnImage.empty()){
			mKeyShiftDnImage = mKeyShiftUpImage;
		}
		if(mKeyDnTextConfig.empty()){
			mKeyDnTextConfig = mKeyUpTextConfig;
		}
	}

	ci::Vec2f			mKeyInitialPosition;

	std::string			mKeyUpTextConfig;
	std::string			mKeyDnTextConfig;
	ci::Vec2f			mKeyTextOffset;
	ci::Color			mKeyDownColor;
	float				mKeyTouchPadding;

	std::string			mKeyLetterUpImage;
	std::string			mKeyLetterDnImage;
	std::string			mKeyNumberUpImage;
	std::string			mKeyNumberDnImage;
	std::string			mKeySpaceUpImage;
	std::string			mKeySpaceDnImage;
	std::string			mKeyDeleteUpImage;
	std::string			mKeyDeleteDnImage;
	std::string			mKeyEnterUpImage;
	std::string			mKeyEnterDnImage;
	std::string			mKeyShiftUpImage;
	std::string			mKeyShiftDnImage;


};

} // namespace ui
} // namespace ds

#endif
