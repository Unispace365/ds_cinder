#include "stdafx.h"

#include "settings_editor.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/cfg/editor_components/editor_item.h"
#include "ds/cfg/editor_components/edit_view.h"

namespace ds{
namespace cfg{

SettingsEditor::SettingsEditor(ds::ui::SpriteEngine& e)
	: ds::ui::Sprite(e)
	, mCurrentSettings(nullptr)
	, mPrimaryLayout(nullptr)
	, mSettingsLayout(nullptr)
	, mSaveLocal(nullptr)
	, mEditView(nullptr)
{
	mPrimaryLayout = new ds::ui::LayoutSprite(mEngine);
	addChildPtr(mPrimaryLayout);
	mPrimaryLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mPrimaryLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mPrimaryLayout->setClipping(true);
	mPrimaryLayout->setSpacing(10.0f);
	mPrimaryLayout->enable(true); // block touches from hitting the app


	ds::ui::Sprite* backgroundSprite = new ds::ui::Sprite(mEngine);
	backgroundSprite->setColor(ci::Color::black());
	backgroundSprite->setOpacity(0.9f);
	backgroundSprite->setTransparent(false);
	backgroundSprite->setBlendMode(ds::ui::BlendMode::MULTIPLY);
	backgroundSprite->mLayoutUserType = ds::ui::LayoutSprite::kFillSize;
	mPrimaryLayout->addChildPtr(backgroundSprite);



		
	mSettingsLayout = new ds::ui::LayoutSprite(mEngine);
	mPrimaryLayout->addChildPtr(mSettingsLayout);
	mSettingsLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mSettingsLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mSettingsLayout->setSpacing(10.0f);
	mSettingsLayout->enable(true);
	mSettingsLayout->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION_Y);
	mSettingsLayout->mLayoutTPad = 10.0f;
	mSettingsLayout->mLayoutRPad = 10.0f;
	mSettingsLayout->mLayoutLPad = 10.0f;
	mSettingsLayout->mLayoutBPad = 10.0f;

	hide();
}

void SettingsEditor::showSettings(Settings* theSettings){
	for (auto it : mSettingItems){
		it->release();
	}
	mCurrentSettings = theSettings;
	mSettingItems.clear();

	if(!mSettingsLayout || !mPrimaryLayout) return;

	show();



	if(!mSaveLocal){
		mSaveLocal = new ds::ui::Text(mEngine);
		mSaveLocal->setFont("Arial Bold");
		mSaveLocal->setFontSize(14.0f);
		mSaveLocal->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
		mSaveLocal->setText("Save in app settings");
		mSaveLocal->enable(true);
		mSaveLocal->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mSaveLocal->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos){
			if(mCurrentSettings){
				mCurrentSettings->writeTo(ds::Environment::expand("%LOCAL%/settings/%PP%/" + mCurrentSettings->getName() + ".xml"));
				mCurrentSettings->writeTo(ds::Environment::expand("%APP%/settings/" + mCurrentSettings->getName() + ".xml"));
			}
		});

		auto theWidth = mSaveLocal->getWidth();
		std::cout << "save base width  " << theWidth << " " << mSaveLocal->getHeight() << std::endl;
		mSettingsLayout->addChildPtr(mSaveLocal);
	}

	theSettings->forEachSetting([this](ds::cfg::Settings::Setting& setting){
		EditorItem* ei = new EditorItem(mEngine);
		Settings::Setting* theSetting = &setting;
		ei->setSetting(theSetting);
		ei->enable(true);
		ei->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		ei->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
			if(bs && mSettingsLayout && ti.mPhase == ds::ui::TouchInfo::Moved && glm::distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()){
				bs->passTouchToSprite(mSettingsLayout, ti);
			}
		});
		ei->setTapCallback([this, ei](ds::ui::Sprite* bs, const ci::vec3& pos){
			editProperty(ei);
		});
		if(mSettingsLayout){
			mSettingsLayout->addChildPtr(ei);
		}
		mSettingItems.push_back(ei);
	});

	mPrimaryLayout->runLayout();

	setPosition(mEngine.getSrcRect().getX2() - mPrimaryLayout->getWidth(), mEngine.getSrcRect().getY1());

}

void SettingsEditor::hideSettings(){
	hide();
	if(mEditView){
		mEditView->stopEditing();
	}
}

void SettingsEditor::editProperty(EditorItem* ei){
	if(ei && !ei->getSettingName().empty()){
		if(!mEditView){
			mEditView = new EditView(mEngine);
			addChildPtr(mEditView);
		}
		mEditView->show();
		Settings::Setting* theSetting = &mCurrentSettings->getSetting(ei->getSettingName(), 0);
		mEditView->setSetting(theSetting);
		mEditView->setPosition(-mEditView->getWidth(), 150.0f);
		mEditView->setRequestNextSettingCallback([this, ei](const bool isNext){
			EditorItem* theNewItem = nullptr;
			if(isNext) theNewItem = getNextItem(ei);
			if(!isNext) theNewItem = getPrevItem(ei);

			if(theNewItem){
				editProperty(theNewItem);
			}
		});

		mEditView->setSettingUpdatedCallback([this](Settings::Setting* theSetting){
			if(!theSetting) return;
			for (auto it : mSettingItems){
				if(it->getSettingName() == theSetting->mName && mSettingsLayout && mPrimaryLayout){
					auto yPos = mSettingsLayout->getPosition().y;
					it->setSetting(theSetting);
					mPrimaryLayout->runLayout();
					mSettingsLayout->setPosition(mSettingsLayout->getPosition().x, yPos);

					break;
				}
			}
		});
	}
}

EditorItem* SettingsEditor::getNextItem(EditorItem* ei){
	if(mSettingItems.empty()) return nullptr;

	EditorItem* nextItem = nullptr;
	bool hasNonHeaders = false;
	bool foundThis = false;
	for(auto it : mSettingItems){
		if(!it->getIsHeader()) hasNonHeaders = true;

		// we found the one we're looking for in the previous loop, use this item
		if(foundThis && !it->getIsHeader()){
			nextItem = it;
			break;
		}
		// this is the current one we're looking for
		if(it->getSettingName() == ei->getSettingName()){
			foundThis = true;
		}
	}

	if(!hasNonHeaders){
		return nullptr;
	}

	if(!nextItem){
		if(mSettingItems.front()->getIsHeader()){
			return getNextItem(mSettingItems.front());
		}
		return mSettingItems.front();
	}

	return nextItem;
}

ds::cfg::EditorItem* SettingsEditor::getPrevItem(EditorItem* ei){
	if(mSettingItems.empty()) return nullptr;

	EditorItem* prevItem = nullptr;
	bool hasNonHeaders = false;
	for(auto it : mSettingItems){
		if(it->getIsHeader()) continue;
		hasNonHeaders = true;

		// this is the current one we're looking for, so return the prev item
		if(it->getSettingName() == ei->getSettingName()){
			break;
		}
		// keep this item till the next loop in case we find it
		prevItem = it;
	}
	
	if(!prevItem){
		if(mSettingItems.back()->getIsHeader()){
			return getPrevItem(mSettingItems.back());
		} 
		return mSettingItems.back();
	}

	return prevItem;
}

}
}
