#pragma once
#ifndef ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS
#define ESSENTIALS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/environment.h>
#include <ds/util/file_meta_data.h>
#include "ds/ui/soft_keyboard/soft_keyboard_button.h"

namespace ds {
namespace ui {
class SoftKeyboardSettings {

public:
	SoftKeyboardSettings()
		: mKeyTouchPadding(4.0f)
		, mKeyInitialPosition(0.0f, 0.0f)
		, mKeyTextOffset(-5.0f, -5.0f)
		, mKeyUpColor(1.0f, 1.0f, 1.0f)
		, mKeyDownColor(0.5f, 0.5f, 0.5f)
		, mKeyLetterDnImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyLetterUpImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyNumberDnImage("%APP%/data/images/keyboard/Normal.png")
		, mKeyNumberUpImage("%APP%/data/images/keyboard/Normal.png")
		, mKeySpaceDnImage("%APP%/data/images/keyboard/Space.png")
		, mKeySpaceUpImage("%APP%/data/images/keyboard/Space.png")
		, mKeyDeleteDnImage("%APP%/data/images/keyboard/Delete.png")
		, mKeyDeleteUpImage("%APP%/data/images/keyboard/Delete.png")
		, mKeyEnterDnImage("%APP%/data/images/keyboard/Enter.png")
		, mKeyEnterUpImage("%APP%/data/images/keyboard/Enter.png")
		, mKeyShiftDnImage("%APP%/data/images/keyboard/Shift.png")
		, mKeyShiftUpImage("%APP%/data/images/keyboard/Shift.png")
		, mKeyTabUpImage("%APP%/data/images/keyboard/Tab.png")
		, mKeyTabDnImage("%APP%/data/images/keyboard/Tab.png")
		, mKeyUpTextConfig("keyboard:key:up")
		, mKeyDnTextConfig("keyboard:key:down")
		, mKeyScale(1.0f)
	{}

	void				normalizeSettings(){
		if(mKeyLetterDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyLetterDnImage))){
			mKeyLetterDnImage = mKeyLetterUpImage;
		}
		if(mKeyNumberUpImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyNumberUpImage))){
			mKeyNumberUpImage = mKeyLetterUpImage;
		}
		if(mKeyNumberDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyNumberDnImage))){
			mKeyNumberDnImage = mKeyNumberUpImage;
		}
		if(mKeySpaceDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeySpaceDnImage))){
			mKeySpaceDnImage = mKeySpaceUpImage;
		}
		if(mKeyDeleteDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyDeleteDnImage))){
			mKeyDeleteDnImage = mKeyDeleteUpImage;
		}

		bool enterUpExists = ds::safeFileExistsCheck(ds::Environment::expand(mKeyEnterUpImage));
		if(!enterUpExists){
			mKeyEnterUpImage = "%APP%/data/images/keyboard/EnterAndShift.png";
		}
		if(mKeyEnterDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyEnterDnImage))){
			mKeyEnterDnImage = mKeyEnterUpImage;
		}

		bool shiftUpExists = ds::safeFileExistsCheck(ds::Environment::expand(mKeyShiftUpImage));
		if(!enterUpExists){
			mKeyShiftUpImage = "%APP%/data/images/keyboard/EnterAndShift.png";
		}
		if(mKeyShiftDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyShiftDnImage))){
			mKeyShiftDnImage = mKeyShiftUpImage;
		}

		if(mKeyDnTextConfig.empty() || mKeyDnTextConfig.empty()){
			mKeyDnTextConfig = mKeyUpTextConfig;
		}
		if(mKeyTabUpImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyTabUpImage))){
			mKeyTabUpImage = mKeyShiftUpImage;
		}
		if(mKeyTabDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyTabDnImage))){
			mKeyTabDnImage = mKeyShiftUpImage;
		}
	}

	ci::Vec2f			mKeyInitialPosition;

	std::string			mKeyUpTextConfig;
	std::string			mKeyDnTextConfig;
	ci::Vec2f			mKeyTextOffset;
	ci::Color			mKeyDownColor;
	ci::Color			mKeyUpColor;
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
	std::string			mKeyTabUpImage;
	std::string			mKeyTabDnImage;

	float				mKeyScale;
};

} // namespace ui
} // namespace ds

#endif
