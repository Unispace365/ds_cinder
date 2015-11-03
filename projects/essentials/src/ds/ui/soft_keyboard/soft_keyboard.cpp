#include "soft_keyboard.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/image.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "ds/ui/soft_keyboard/soft_keyboard_button.h"

namespace ds{
namespace ui{

SoftKeyboard::SoftKeyboard(ds::ui::SpriteEngine& engine, SoftKeyboardSettings& settings)
	: ds::ui::Sprite(engine)
	, mSoftKeyboardSettings(settings)
	, mUpperCase(false)
{
	mSoftKeyboardSettings.normalizeSettings();
}

void SoftKeyboard::handleKeyPress(SoftKeyboardButton* key){

	std::string currentCharacter = key->getCharacter();
	const SoftKeyboardButton::KeyType keyType = key->getKeyType();
	if(keyType == SoftKeyboardButton::kDelete){
		if(mCurrentText.length() > 0){
			mCurrentText = mCurrentText.substr(0, mCurrentText.length() - 1);
		}
		if(mKeyPressFunction) mKeyPressFunction(currentCharacter, keyType);
	} else if(keyType == SoftKeyboardButton::kShift){
		toggleShift();
		if(mKeyPressFunction) mKeyPressFunction("", keyType);
	} else if(keyType == SoftKeyboardButton::kEnter){
		if(mKeyPressFunction) mKeyPressFunction("", keyType);
	} else if(keyType == SoftKeyboardButton::kSpace){
		std::stringstream ss;
		ss << mCurrentText << " ";
		mCurrentText = ss.str();
		if(mKeyPressFunction) mKeyPressFunction(" ", keyType);
	} else {
		std::stringstream ss;
		ss << mCurrentText << currentCharacter;
		mCurrentText = ss.str();
		if(mKeyPressFunction) mKeyPressFunction(currentCharacter, keyType);
	}
}

void SoftKeyboard::setKeyPressFunction(const std::function<void(const std::string& character, const SoftKeyboardButton::KeyType keyType)>& func) {
	mKeyPressFunction = func;
}

void SoftKeyboard::toggleShift() {
	mUpperCase = !mUpperCase;
	for(auto it = mButtons.begin(); it < mButtons.end(); ++it){
		(*it)->setToggle(mUpperCase);
	}
}

const std::string& SoftKeyboard::getCurrentText(){
	return mCurrentText;
}

void SoftKeyboard::setCurrentText(const std::string& crTxStr){
	mCurrentText = crTxStr;
}

void SoftKeyboard::resetCurrentText() {
	mCurrentText = "";
}


}
}