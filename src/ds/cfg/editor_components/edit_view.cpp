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

	ds::ui::Sprite* backgroundSprite = new ds::ui::Sprite(mEngine);
	backgroundSprite->setColor(ci::Color::black());
	backgroundSprite->setOpacity(0.9f);
	backgroundSprite->setTransparent(false);
	backgroundSprite->setBlendMode(ds::ui::BlendMode::MULTIPLY);
	backgroundSprite->mLayoutUserType = ds::ui::LayoutSprite::kFillSize;
	addChildPtr(backgroundSprite);


	mSettingName = addTextSprite("Arial Narrow", 21.0f, 1.0f);
	mSettingName->setColor(ci::Color(0.9f, 0.282f, 0.035f));

	mSettingValue = addTextSprite("Arial Bold", 21.0f, 1.0f);
	mSettingComment = addTextSprite("Arial Narrow", 14.0f, 0.4f);
	mSettingDefault = addTextSprite("Arial Narrow", 14.0f, 0.4f);
	mSettingMin = addTextSprite("Arial Narrow", 14.0f, 0.4f);
	mSettingMax = addTextSprite("Arial Narrow", 14.0f, 0.4f);
	mSettingSource = addTextSprite("Arial Narrow", 14.0f, 0.4f);

	mApplyButton = addTextSprite("Arial Narrow", 18.0f, 1.0f);
	mApplyButton->setColor(ci::Color(0.9f, 0.282f, 0.035f));
	mApplyButton->enable(true);
	mApplyButton->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	mApplyButton->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		mEngine.getNotifier().notify(ds::cfg::Settings::SettingsEditedEvent("", ""));
	});

}

ds::ui::Text* EditView::addTextSprite(const std::string& fontName, const float fontSize, const float opacity){
	ds::ui::Text* newText = new ds::ui::Text(mEngine);
	newText->setFont(fontName);
	newText->setFontSize(fontSize);
	newText->setOpacity(opacity);
	newText->mLayoutTPad = 5.0f;
	newText->mLayoutLPad = 5.0f;
	newText->mLayoutRPad = 5.0f;
	newText->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	addChildPtr(newText);
	return newText;
}

void EditView::setSetting(Settings::Setting* theSetting){
	mTheSetting = theSetting;

	if(!mSettingName || !mSettingValue || !mTheSetting || !mSettingComment || !mSettingDefault || !mSettingMin || !mSettingMax || !mSettingSource) return;


	mSettingName->setText(theSetting->mName);
	mSettingValue->setText(theSetting->mRawValue);	
	mSettingComment->setText("Comment: " + theSetting->mComment);
	mSettingDefault->setText("Default: " + theSetting->mDefault);
	mSettingMin->setText("Min: " + theSetting->mMinValue);
	mSettingMax->setText("Max: " + theSetting->mMaxValue);
	mSettingSource->setText("Source: " + theSetting->mSource);

	if(!mEntryEditor){
		ds::ui::EntryFieldSettings efs;
		efs.mTextConfig = mEngine.getEngineCfg().getDefaultTextCfgName();
		mEntryEditor = new ds::ui::EntryField(mEngine, efs);
		mEntryEditor->mLayoutTPad = 5.0f;
		mEntryEditor->mLayoutLPad = 5.0f;
		mEntryEditor->mLayoutRPad = 5.0f;
		mEntryEditor->mLayoutBPad = 5.0f;
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
		mKeyboard->mLayoutTPad = 5.0f;
		mKeyboard->mLayoutLPad = 5.0f;
		mKeyboard->mLayoutRPad = 5.0f;
		mKeyboard->mLayoutBPad = 10.0f;
	}

	if(mEntryEditor && mKeyboard){
		mKeyboard->setKeyPressFunction([this](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
			if(mEntryEditor){
				mEntryEditor->keyPressed(character, keyType);
			}
		});

		mEntryEditor->setTextUpdatedCallback([this](const std::wstring& line){
			if(mSettingValue){
				mSettingValue->setText(line);
			}
		});

		mEntryEditor->setCurrentText(mSettingValue->getText());
		mEntryEditor->autoRegisterOnFocus(true);
		mEntryEditor->focus();
	}

	show();
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
