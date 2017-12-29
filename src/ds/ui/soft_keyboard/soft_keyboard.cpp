#include "stdafx.h"

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
	, mCurrentText(L"")
{
	mSoftKeyboardSettings.normalizeSettings();

	mLayoutFixedAspect = true;
}

void SoftKeyboard::handleKeyPress(SoftKeyboardButton* key){

	std::wstring currentCharacter = key->getCharacter();
	const SoftKeyboardDefs::KeyType keyType = key->getKeyType();
	if(keyType == SoftKeyboardDefs::kShift){
		toggleShift();
	} 

	handleKeyPressGeneric(keyType, currentCharacter, mCurrentText);
	if(mKeyPressFunction) mKeyPressFunction(currentCharacter, keyType);
}

void SoftKeyboard::setSoftKeyboardSettings(SoftKeyboardSettings& newSettings) {
	mSoftKeyboardSettings = newSettings;
	for (auto it :mButtons){
		it->setSoftKeyboardSettings(mSoftKeyboardSettings);
	}
}

void SoftKeyboard::setKeyPressFunction(const std::function<void(const std::wstring& character, const SoftKeyboardDefs::KeyType keyType)>& func) {
	mKeyPressFunction = func;
}

void SoftKeyboard::toggleShift() {
	mUpperCase = !mUpperCase;
	for(auto it = mButtons.begin(); it < mButtons.end(); ++it){
		(*it)->setShifted(mUpperCase);
	}
}

const std::wstring& SoftKeyboard::getCurrentText(){
	return mCurrentText;
}

void SoftKeyboard::setCurrentText(const std::wstring& crTxStr){
	mCurrentText = crTxStr;
}

void SoftKeyboard::resetCurrentText() {
	mCurrentText = L"";
}


}
}