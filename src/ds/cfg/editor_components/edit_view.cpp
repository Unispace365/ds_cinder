#include "stdafx.h"

#include "edit_view.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"

#include <cinder/TriMesh.h>
#include <cinder/Triangulate.h>

#include <ds/ui/soft_keyboard/soft_keyboard_builder.h>

#include "ds/cfg/settings.h"

namespace ds{
namespace cfg{

EditView::EditView(ds::ui::SpriteEngine& e)
	: ds::ui::LayoutSprite(e)
	, mTheSetting(nullptr)
	, mSettingValue(nullptr)
	, mSettingName(nullptr)
	, mSettingComment(nullptr)
	, mSettingDefault(nullptr)
	, mSettingMin(nullptr)
	, mSettingMax(nullptr)
	, mSettingSource(nullptr)
	, mApplyButton(nullptr)
	, mEntryEditor(nullptr)
	, mKeyboard(nullptr)
{
	setSize(600.0f, 200.0f);
	setShrinkToChildren(ds::ui::LayoutSprite::kShrinkHeight);
	setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	enable(true); // block touchies

	ds::ui::Sprite* backgroundSprite = new ds::ui::Sprite(mEngine);
	backgroundSprite->setColor(ci::Color::black());
	backgroundSprite->setOpacity(0.9f);
	backgroundSprite->setTransparent(false);
	backgroundSprite->setBlendMode(ds::ui::BlendMode::MULTIPLY);
	backgroundSprite->mLayoutUserType = ds::ui::LayoutSprite::kFillSize;
	addChildPtr(backgroundSprite);


	mSettingName = addTextSprite("Arial Bold", 24.0f, 1.0f, false);
	mSettingName->setColor(ci::Color(0.9f, 0.282f, 0.035f));

	mSettingValue = addTextSprite("Arial Bold", 21.0f, 1.0f, false);
	mSettingComment = addTextSprite("Arial", 14.0f, 0.75f, false);
	mSettingDefault = addTextSprite("Arial", 14.0f, 0.4f, true);

	mSettingMin = addTextSprite("Arial", 14.0f, 0.4f, true);
	mSettingMax = addTextSprite("Arial", 14.0f, 0.4f, true);
	mSettingSource = addTextSprite("Arial", 14.0f, 0.4f, false);

	mApplyButton = addTextSprite("Arial Narrow", 18.0f, 1.0f, true);
	mApplyButton->setColor(ci::Color(0.9f, 0.282f, 0.035f));
	mApplyButton->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		if(mSettingUpdatedCalback){
			mSettingUpdatedCalback(mTheSetting);
		}
		mEngine.getNotifier().notify(ds::cfg::Settings::SettingsEditedEvent(mParentSettingsName, mTheSetting->mName));
	});
	mApplyButton->mLayoutHAlign = ds::ui::LayoutSprite::kRight;
	mApplyButton->mLayoutBPad = 10.0f;
	mApplyButton->mLayoutRPad = 10.0f;
	mApplyButton->setText("Apply");

}

ds::ui::Text* EditView::addTextSprite(const std::string& fontName, const float fontSize, const float opacity, const bool clickSetValue){
	ds::ui::Text* newText = new ds::ui::Text(mEngine);
	newText->setFont(fontName);
	newText->setFontSize(fontSize);
	newText->setOpacity(opacity);
	newText->mLayoutTPad = 5.0f;
	newText->mLayoutLPad = 5.0f;
	newText->mLayoutRPad = 5.0f;
	newText->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	if(clickSetValue){
		newText->enable(true);
		newText->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		newText->setTapCallback([this, newText](ds::ui::Sprite* bs, const ci::vec3& pos){
			std::wstring theText = newText->getText();
			auto splitty = ds::split(theText, L": ", true);
			if(mEntryEditor && splitty.size() > 1){
				mEntryEditor->setCurrentText(splitty[1]);
				updateValue(splitty[1]);
			}
		});
	}
	addChildPtr(newText);
	return newText;
}

void EditView::setSetting(Settings::Setting* theSetting, const std::string& parentSetingsName){
	mTheSetting = theSetting;
	mParentSettingsName = parentSetingsName;

	if(!mSettingName || !mSettingValue || !mTheSetting || !mSettingComment || !mSettingDefault || !mSettingMin || !mSettingMax || !mSettingSource) return;


	mSettingName->setText(theSetting->mName);
	//mSettingValue->setText(theSetting->mRawValue);	
	mSettingComment->setText(theSetting->mComment);
	mSettingDefault->setText("Default: " + theSetting->mDefault);
	mSettingMin->setText("Min: " + theSetting->mMinValue);
	mSettingMax->setText("Max: " + theSetting->mMaxValue);
	mSettingSource->setText("Source: " + theSetting->mSource);

	if(!mEntryEditor){
		ds::ui::EntryFieldSettings efs;
		efs.mFieldSize = ci::vec2(600.0f, 40.0f);
		efs.mTextConfig = mEngine.getEngineCfg().getDefaultTextCfgName();
		mEntryEditor = new ds::ui::EntryField(mEngine, efs);
		mEntryEditor->mLayoutTPad = 5.0f;
		mEntryEditor->mLayoutLPad = 5.0f;
		mEntryEditor->mLayoutRPad = 5.0f;
		mEntryEditor->mLayoutBPad = 0.0f;
		addChildPtr(mEntryEditor);
	}

	if(!mKeyboard){
		ds::ui::SoftKeyboardSettings sks;
		sks.mKeyDnTextConfig = mEngine.getEngineCfg().getDefaultTextCfgName();
		sks.mKeyUpTextConfig = mEngine.getEngineCfg().getDefaultTextCfgName();
		sks.mKeyUpColor = ci::Color::white();
		sks.mKeyScale = 0.5f;
		sks.normalizeSettings();
		mKeyboard = ds::ui::SoftKeyboardBuilder::buildExtendedKeyboard(mEngine, sks, this);
		mKeyboard->mLayoutTPad = 0.0f;
		mKeyboard->mLayoutLPad = 5.0f;
		mKeyboard->mLayoutRPad = 5.0f;
		mKeyboard->mLayoutBPad = 10.0f;
	}

	if(mEntryEditor && mKeyboard){
		mKeyboard->setKeyPressFunction([this](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
			if(mEntryEditor){
				if(keyType == ds::ui::SoftKeyboardDefs::kEnter){
					stopEditing();
				} else {
					mEntryEditor->keyPressed(character, keyType);
				}
			}
		});

		mEntryEditor->setNativeKeyboardCallback([this](ci::app::KeyEvent& event)->bool{
			if(event.getCode() == ci::app::KeyEvent::KEY_ESCAPE){
				stopEditing();
				return true;
			}

			if(mNextSettingCallback &&
			   (event.getCode() == ci::app::KeyEvent::KEY_DOWN
			   || event.getCode() == ci::app::KeyEvent::KEY_TAB
			   || event.getCode() == ci::app::KeyEvent::KEY_RETURN
			   || event.getCode() == ci::app::KeyEvent::KEY_KP_ENTER
			   )){
				mNextSettingCallback(true);
				return true;
			}

			if(mNextSettingCallback &&
			   event.getCode() == ci::app::KeyEvent::KEY_UP){
				mNextSettingCallback(false);
				return true;
			}

			return false;

		});

		mEntryEditor->setTextUpdatedCallback([this](const std::wstring& line){
			updateValue(line);
			//mEngine.getNotifier().notify(Settings::SettingsEditedEvent("", ""));
		});

		if(mTheSetting) mEntryEditor->setCurrentText(ds::wstr_from_utf8(mTheSetting->mRawValue));
		mEntryEditor->autoRegisterOnFocus(true);
		mEntryEditor->focus();
	}

	if(mApplyButton){
		mApplyButton->sendToFront();
	}
	show();
	runLayout();

}

void EditView::updateValue(const std::wstring& theValue){
	if(mSettingValue){
		//	mSettingValue->setText(line);
	}

	if(mTheSetting){
		mTheSetting->mRawValue = ds::utf8_from_wstr(theValue);

	}

	runLayout();
}

void EditView::stopEditing(){

	hide();
	if(mEntryEditor){
		mEntryEditor->unfocus();
	}
}

}
}
