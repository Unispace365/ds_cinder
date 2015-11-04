#include "entry_field.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/ui/sprite/image.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "ds/ui/soft_keyboard/soft_keyboard_button.h"

namespace ds{
namespace ui{

EntryField::EntryField(ds::ui::SpriteEngine& engine, EntryFieldSettings& settings)
	: ds::ui::Sprite(engine)
	, mCursor(nullptr)
	, mTextSprite(nullptr)
	, mInFocus(false)
{
	mTextSprite = new ds::ui::MultilineText(engine);
	mTextSprite->setResizeLimit(500.0f, 100.0f);
	addChildPtr(mTextSprite);

	mCursor = new ds::ui::Sprite(engine);
	mCursor->setTransparent(false);
	addChildPtr(mCursor);

	setEntryFieldSettings(settings);
}

void EntryField::setEntryFieldSettings(EntryFieldSettings& newSettings){
	mEntryFieldSettings = newSettings;

	if(!newSettings.mTextConfig.empty()){
		mEngine.getEngineCfg().getText(newSettings.mTextConfig).configure(*mTextSprite);
	}

	if(mCursor){
		mCursor->setColor(newSettings.mCursorColor);
		mCursor->setSize(newSettings.mCursorSize);
	}
}

const std::wstring EntryField::getCurrentText(){
	if(mTextSprite){
		return mTextSprite->getText();
	}
	return L"";
}

void EntryField::setCurrentText(const std::wstring& crTxStr){
	if(mTextSprite){
		mTextSprite->setText(crTxStr);
	}
	
	mCursorIndex = crTxStr.size();

	textUpdated();
}

void EntryField::keyPressed(const std::wstring& keyCharacter, const ds::ui::SoftKeyboardDefs::KeyType keyType){
	std::wstring currentCharacter = keyCharacter;
	std::wstring currentFullText = getCurrentText();
	handleKeyPressGeneric(keyType, currentCharacter, currentFullText);

	setCurrentText(currentFullText);
}


void EntryField::resetCurrentText() {
	setCurrentText(L"");
}

void EntryField::focus(){
	if(mInFocus) return;
	mInFocus = true;

	if(mCursor){
		mCursor->show();
		mCursor->setOpacity(0.0f);
		blinkCursor();
	}

	onFocus();
}

void EntryField::blinkCursor(){
	if(!mCursor) return;
	mCursor->tweenOpacity(1.0f, mEntryFieldSettings.mAnimationRate, 0.0f, ci::easeNone, [this]{
		mCursor->tweenOpacity(1.0f, mEntryFieldSettings.mBlinkRate, 0.0f, ci::easeNone, [this]{
			mCursor->tweenOpacity(0.0f, mEntryFieldSettings.mAnimationRate, 0.0f, ci::easeNone, [this]{
				blinkCursor();
			});
		});
	});

}

void EntryField::unfocus(){
	if(!mInFocus) return;
	mInFocus = false;

	if(mCursor){
		mCursor->tweenOpacity(0.0f, mEntryFieldSettings.mAnimationRate, 0.0f, ci::easeNone, [this]{
			mCursor->hide();
		});
	}
}

void EntryField::textUpdated(){
	// set cursor

	if(mTextSprite && mCursor){
		ci::Vec2f cursorPos = mTextSprite->getPositionForCharacterIndex(mCursorIndex);
		mCursor->setPosition(cursorPos.x + mEntryFieldSettings.mCursorPadding, cursorPos.y);
		setSize(mTextSprite->getWidth(), mTextSprite->getHeight());
	}

	onTextUpdated();
}

void EntryField::setSelectable(const bool isSelectable){
	if(isSelectable){
		enable(true);
		enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	} else {
		enable(false);

	}
}

}
}