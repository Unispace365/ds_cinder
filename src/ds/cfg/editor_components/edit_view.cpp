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
	, mSettingPossibles(nullptr)
	, mSettingSource(nullptr)
	, mCancelButton(nullptr)
	, mApplyButton(nullptr)
	, mButtonHolder(nullptr)
	, mEntryEditor(nullptr)
	, mKeyboard(nullptr)
	, mSlider(nullptr)
	, mPossibleIndex(0)
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
	mSettingComment = addTextSprite("Arial", 12.0f, 0.75f, false);
	mSettingDefault = addTextSprite("Arial", 12.0f, 0.4f, true);

	mSettingMin = addTextSprite("Arial", 12.0f, 0.4f, true);
	mSettingMax = addTextSprite("Arial", 12.0f, 0.4f, true);
	mSettingPossibles = addTextSprite("Arial", 12.0f, 0.4f, false);
	mSettingSource = addTextSprite("Arial", 12.0f, 0.4f, false);

	mButtonHolder = new ds::ui::LayoutSprite(mEngine);
	mButtonHolder->setLayoutType(ds::ui::LayoutSprite::kLayoutHFlow);
	mButtonHolder->mLayoutHAlign = ds::ui::LayoutSprite::kRight;
	mButtonHolder->mLayoutBPad = 10.0f;
	mButtonHolder->mLayoutRPad = 10.0f;
	mButtonHolder->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mButtonHolder->setSpacing(10.0f);
	addChildPtr(mButtonHolder);

	mCancelButton = addTextSprite("Arial Narrow", 18.0f, 0.8f, true);
	mCancelButton->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		stopEditing();
	});
	mCancelButton->setText("Cancel");
	mButtonHolder->addChildPtr(mCancelButton);

	mApplyButton = addTextSprite("Arial Narrow", 18.0f, 1.0f, true);
	mApplyButton->setColor(ci::Color(0.9f, 0.282f, 0.035f));
	mApplyButton->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
		applySetting();
	});
	mApplyButton->setText("Apply");
	mButtonHolder->addChildPtr(mApplyButton);

}

void EditView::applySetting(){
	if(mTheSetting && mEntryEditor){
		mTheSetting->mRawValue = ds::utf8_from_wstr(mEntryEditor->getCurrentText());
	}
	if(mSettingUpdatedCalback){
		mSettingUpdatedCalback(mTheSetting);
	}
	mEngine.getNotifier().notify(ds::cfg::Settings::SettingsEditedEvent(mParentSettingsName, mTheSetting->mName));
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

	if(!mSettingName || !mSettingValue || !mTheSetting || !mSettingComment || !mSettingDefault || !mSettingMin || !mSettingMax || !mSettingSource || !mSettingPossibles) return;


	mSettingName->setText(theSetting->mName);
	//mSettingValue->setText(theSetting->mRawValue);	
	mSettingComment->setText(theSetting->mComment);
	mSettingDefault->setText("Default: " + theSetting->mDefault);
	mSettingMin->setText("Min: " + theSetting->mMinValue);
	mSettingMax->setText("Max: " + theSetting->mMaxValue);
	mSettingPossibles->setText("Possible Values: " + theSetting->mPossibleValues);
	mSettingSource->setText("Source: " + theSetting->mSource);

	if(!mSlider){
		mSlider = new ds::ui::ControlSlider(mEngine, false);
		mSlider->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
		mSlider->mLayoutLPad = 5.0f;
		mSlider->mLayoutRPad = 5.0f;
		mSlider->setSliderInterpolation(ds::ui::ControlSlider::kSliderTypeQuadratic);

		mSlider->setSliderUpdatedCallback([this](double sliderPercent, double sliderValue, bool finishedAdjusting){
			if(mTheSetting && mEntryEditor){
				if(mTheSetting->mType == ds::cfg::SETTING_TYPE_INT){
					int theActualValue = static_cast<int>(round(sliderValue));
					mEntryEditor->setCurrentText(std::to_wstring(theActualValue));
				} else if(mTheSetting->mType == ds::cfg::SETTING_TYPE_FLOAT){
					float theActualValue = static_cast<float>(sliderValue);
					mEntryEditor->setCurrentText(std::to_wstring(theActualValue));
				} else {
					mEntryEditor->setCurrentText(std::to_wstring(sliderValue));
				}
			}
		});
		addChildPtr(mSlider);
	}

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

	if(mEntryEditor && mKeyboard && mSlider && mTheSetting){

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
			   )){
				applySetting();
				mNextSettingCallback(true);
				return true;
			}

			if(mNextSettingCallback &&
			   (event.getCode() == ci::app::KeyEvent::KEY_RETURN
			   || event.getCode() == ci::app::KeyEvent::KEY_KP_ENTER
			   )){
				applySetting();
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

		mEntryEditor->setCurrentText(ds::wstr_from_utf8(mTheSetting->mRawValue));

		mPossibleValues = theSetting->getPossibleValues();
		mPossibleIndex = 0;

		if(theSetting->mType == ds::cfg::SETTING_TYPE_BOOL){
			mEntryEditor->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
				if(mEntryEditor->getCurrentText() == L"true"){
					mEntryEditor->setCurrentText(L"false");
				} else {
					mEntryEditor->setCurrentText(L"true");
				}
			});

			mKeyboard->hide();
			mSlider->hide();
		} else if(mPossibleValues.size() > 1){
			for(int i = 0; i < mPossibleValues.size(); i++){
				if(theSetting->mRawValue == mPossibleValues[i]){
					mPossibleIndex = i;
					break;
				}
			}

			mEntryEditor->setTapCallback([this](ds::ui::Sprite*, const ci::vec3&){
				if(mPossibleValues.size() < 1) return;
				mPossibleIndex++;
				if(mPossibleIndex > mPossibleValues.size() - 1){
					mPossibleIndex = 0;
				}
				if(mPossibleIndex < 0) mPossibleIndex = 0;
				mEntryEditor->setCurrentText(ds::wstr_from_utf8(mPossibleValues[mPossibleIndex]));
			});

			mKeyboard->show();
			mSlider->hide();

		} else if(  (theSetting->mType == ds::cfg::SETTING_TYPE_DOUBLE
				  || theSetting->mType == ds::cfg::SETTING_TYPE_FLOAT
				  || theSetting->mType == ds::cfg::SETTING_TYPE_INT)
				  && !theSetting->mMinValue.empty() && !theSetting->mMaxValue.empty()
				  ){
			mSlider->show();
			mSlider->setSliderLimits(ds::string_to_double(theSetting->mMinValue), ds::string_to_double(theSetting->mMaxValue));
			mSlider->setSliderValue(ds::wstring_to_double(mEntryEditor->getCurrentText()));
		} else {
			mEntryEditor->setTapCallback(nullptr);
			mKeyboard->show();
			mSlider->hide();
		}

		mEntryEditor->autoRegisterOnFocus(true);
		mEntryEditor->focus();
	}

	if(mButtonHolder){
		mButtonHolder->sendToFront();
	}
	show();
	runLayout();

}

void EditView::updateValue(const std::wstring& theValue){
	if(mSettingValue){
		//	mSettingValue->setText(line);
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
