#include "soft_keyboard_button.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

namespace ds {
namespace ui {
SoftKeyboardButton::SoftKeyboardButton(ds::ui::SpriteEngine& engine, const std::wstring& characterlow, const std::wstring& characterUp, const SoftKeyboardDefs::KeyType keyType, SoftKeyboardSettings& settings)
	: ds::ui::ImageButton(engine, "", "", 0.0f)
	, mCharacterLower(characterlow)
	, mCharacterUpper(characterUp)
	, mUpper(false)
	, mKeyType(keyType)
	, mText(nullptr)
	, mPressed(false)
{
	mText = new ds::ui::Text(mEngine);
	addChildPtr(mText);

	setCenter(0.5f, 0.5f);

	setStateChangeFn([this](const bool pressed){
		stateChanged(pressed);
	});

	setSoftKeyboardSettings(settings);
}

const std::wstring& SoftKeyboardButton::getCharacter() {
	if(mUpper) return mCharacterUpper;
	else return mCharacterLower;
}

const SoftKeyboardDefs::KeyType& SoftKeyboardButton::getKeyType() {
	return mKeyType;
}

void SoftKeyboardButton::setSoftKeyboardSettings(SoftKeyboardSettings& softKeySettings) {
	if(mText){
		mTextConfigUp = softKeySettings.mKeyUpTextConfig;
		mTextConfigDown = softKeySettings.mKeyDnTextConfig;
		mTextOffset = softKeySettings.mKeyTextOffset;
	}

	if(mKeyType == SoftKeyboardDefs::kNumber){
		getNormalImage().setImageFile(softKeySettings.mKeyNumberUpImage);
		getHighImage().setImageFile(softKeySettings.mKeyNumberDnImage);
	} else if(mKeyType == SoftKeyboardDefs::kLetter){
		getNormalImage().setImageFile(softKeySettings.mKeyLetterUpImage);
		getHighImage().setImageFile(softKeySettings.mKeyLetterDnImage);
	} else if(mKeyType == SoftKeyboardDefs::kSpace){
		getNormalImage().setImageFile(softKeySettings.mKeySpaceUpImage);
		getHighImage().setImageFile(softKeySettings.mKeySpaceDnImage);
	} else if(mKeyType == SoftKeyboardDefs::kDelete){
		getNormalImage().setImageFile(softKeySettings.mKeyDeleteUpImage);
		getHighImage().setImageFile(softKeySettings.mKeyDeleteDnImage);
	} else if(mKeyType == SoftKeyboardDefs::kShift){
		getNormalImage().setImageFile(softKeySettings.mKeyShiftUpImage);
		getHighImage().setImageFile(softKeySettings.mKeyShiftDnImage);
	} else if(mKeyType == SoftKeyboardDefs::kEnter){
		getNormalImage().setImageFile(softKeySettings.mKeyEnterUpImage);
		getHighImage().setImageFile(softKeySettings.mKeyEnterDnImage);
	} else {
		DS_LOG_WARNING("Warning: key type not supported in SoftKeyboardButton");
	}

	getHighImage().setColor(softKeySettings.mKeyDownColor);

	setTouchPad(softKeySettings.mKeyTouchPadding);

	layout();
}

void SoftKeyboardButton::setShifted(const bool upper) {
	mUpper = upper;

	layout();
}

void SoftKeyboardButton::stateChanged(const bool pressed) {
	mPressed = pressed;

	layout();
}

void SoftKeyboardButton::layout(){
	if(mText){
		if(mPressed){
			if(!mTextConfigDown.empty()){
				mEngine.getEngineCfg().getText(mTextConfigDown).configure(*mText);
			}
		} else {
			if(!mTextConfigUp.empty()){
				mEngine.getEngineCfg().getText(mTextConfigUp).configure(*mText);
			}
		}

		if(mUpper){
			mText->setText(mCharacterUpper);
		} else {
			mText->setText(mCharacterLower);
		}

		mText->setPosition(getWidth() / 2.0f - mText->getWidth() / 2.0f + mTextOffset.x, getHeight() / 2.0f - mText->getHeight() / 2.0f + mTextOffset.y);
	}
}

} // namespace ui
} // namespace ds