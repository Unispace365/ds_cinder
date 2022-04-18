#pragma once
#ifndef DS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS
#define DS_UI_SOFT_KEYBOARD_SOFT_KEYBOARD_SETTINGS

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/environment.h>
#include <ds/util/file_meta_data.h>

namespace ds {
namespace ui {
class SoftKeyboardSettings {

public:


	typedef enum { kBorder, kSolid, kCircularBorder, kCircularSolid } GraphicType;

	SoftKeyboardSettings()
		: mKeyTouchPadding(4.0f)
		, mKeyInitialPosition(0.0f, 0.0f)
		, mKeyTextOffset(0.0f, 0.0f)
		, mKeyUpColor(1.0f, 1.0f, 1.0f)
		, mKeyDownColor(0.5f, 0.5f, 0.5f)
		, mGraphicKeys(false)
		, mGraphicType(kBorder)
		, mGraphicKeySize(64.0f)
		, mGraphicBorderWidth(1.0f)
		, mGraphicRoundedCornerRadius(0.0f)
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
		, mEmailMode(false) // Only applies to the "extended" keyboard (adds @ and .com keys)
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

		if(mKeyDnTextConfig.empty()){
			mKeyDnTextConfig = mKeyUpTextConfig;
		}
		if(mKeyTabUpImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyTabUpImage))){
			mKeyTabUpImage = mKeyShiftUpImage;
		}
		if(mKeyTabDnImage.empty() || !ds::safeFileExistsCheck(ds::Environment::expand(mKeyTabDnImage))){
			mKeyTabDnImage = mKeyShiftUpImage;
		}
	}

	ci::vec2			mKeyInitialPosition;

	std::string			mKeyUpTextConfig;
	std::string			mKeyDnTextConfig;
	ci::vec2			mKeyTextOffset;
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

	bool				mEmailMode;

	/// Graphic keys uses sprite graphics instead of images
	bool				mGraphicKeys;
	/// The type of graphic style  kBorder, kSolid, kCircularBorder, kCircularSolid
	GraphicType			mGraphicType;
	/// The size of keys when it's graphical
	float				mGraphicKeySize;
	/// Rounded corner radius when it's Border or Solid (doesn't apply to circular)
	float				mGraphicRoundedCornerRadius;
	/// How wide the border graphic is when not a Solid type
	float				mGraphicBorderWidth;

	/// For image keys, scales the images up or down
	float				mKeyScale;
};

} // namespace ui
} // namespace ds

#endif
