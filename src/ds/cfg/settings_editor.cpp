#include "stdafx.h"

#include "settings_editor.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

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
	, mTitle(nullptr)
	, mSaveApp(nullptr)
	, mSaveLocal(nullptr)
	, mSaveConfig(nullptr)
	, mSaveLocalConfig(nullptr)
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

void SettingsEditor::showSettings(const std::string theSettingsName){
	for (auto it : mSettingItems){
		it->release();
	}

	mCurrentSettings = &mEngine.getEngineCfg().getSettings(theSettingsName);
	mSettingItems.clear();

	if(!mSettingsLayout || !mPrimaryLayout) return;

	show();

	if(!mTitle){
		mTitle = new ds::ui::Text(mEngine);
		mTitle->setFont("Arial Narrow");
		mTitle->setFontSize(24.0f);
		mSettingsLayout->addChildPtr(mTitle);

		mTitle->enable(true);
		mTitle->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mTitle->setTapCallback([this](ds::ui::Sprite*, const ci::vec3&){
			try{
				if(mCurrentSettings && !mCurrentSettings->getName().empty()){
					std::string thisSettingName = mCurrentSettings->getName();
					std::string newSettingsName = mEngine.getEngineCfg().getNextSettings(thisSettingName).getName();
					
					showSettings(newSettingsName);
				}
			} catch(std::exception& e){
				std::cout << "Oh whoops " << e.what() << std::endl;
			}
		});
	}

	if(mTitle){
		mTitle->setText(mCurrentSettings->getName() + " >");
	}

	if(!mSaveApp) mSaveApp = createSaveButton(mSettingsLayout);
	if(!mSaveLocal) mSaveLocal = createSaveButton(mSettingsLayout);
	if(!mSaveConfig) mSaveConfig = createSaveButton(mSettingsLayout);
	if(!mSaveLocalConfig) mSaveLocalConfig = createSaveButton(mSettingsLayout);

	setSaveTouch(mSaveApp, "%APP%/settings/" + mCurrentSettings->getName() + ".xml");
	setSaveTouch(mSaveLocal, "%LOCAL%/settings/%PP%/" + mCurrentSettings->getName() + ".xml");
	setSaveTouch(mSaveConfig, "%APP%/settings/%CFG_FOLDER%/" + mCurrentSettings->getName() + ".xml");
	setSaveTouch(mSaveLocalConfig, "%LOCAL%/settings/%PP%/%CFG_FOLDER%/" + mCurrentSettings->getName() + ".xml");

	mCurrentSettings->forEachSetting([this](ds::cfg::Settings::Setting& setting){
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
	mPrimaryLayout->setScale(mEngine.getSrcRect().getWidth() / mEngine.getDstRect().getWidth());
	//setPosition(ci::app::getWindowBounds().getWidth() - mPrimaryLayout->getWidth(), 0.0f);// mEngine.getSrcRect().getY1());
	setPosition(mEngine.getSrcRect().getX2() - mPrimaryLayout->getScaleWidth(), mEngine.getSrcRect().getY1());

}

ds::ui::Text* SettingsEditor::createSaveButton(ds::ui::LayoutSprite* theParent){
	auto newSaveButton = new ds::ui::Text(mEngine);
	newSaveButton->mLayoutLPad = 10.0f;
	newSaveButton->setFont("Arial");
	newSaveButton->setFontSize(14.0f);
	newSaveButton->setColor(ci::Color(0.8f, 0.2f, 0.2f));
	newSaveButton->enable(true);
	newSaveButton->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	theParent->addChildPtr(newSaveButton);
	return newSaveButton;
}

void SettingsEditor::setSaveTouch(ds::ui::Text* theButton, const std::string& saveName){
	if(!theButton) return;

	std::string savePath = ds::Environment::expand(saveName);
	theButton->setText("Save in " + saveName);
	theButton->setTapCallback([this, savePath, saveName, theButton](ds::ui::Sprite* bs, const ci::vec3& pos){
		if(mCurrentSettings){
			mCurrentSettings->writeTo(savePath);

#ifdef _WIN32
			/// open the parent folder in windows explorer
			try{
				Poco::Path thePath = Poco::Path(savePath);
				thePath.makeParent();
				ShellExecute(NULL, L"open", ds::wstr_from_utf8(thePath.toString()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
			} catch(std::exception){
				// just making sure we don't crash, this isn't a required thing
			}
#endif

			theButton->setText("Saved!");
			theButton->callAfterDelay([this, saveName, theButton]{
				theButton->setText("Save in " + saveName);
			}, 3.0f);
		}

	});
	// get the width so the text gets measured so layouts work ok
	auto theWidth = theButton->getWidth();
}

void SettingsEditor::hideSettings(){
	hide();
	if(mEditView){
		mEditView->stopEditing();
	}
}

void SettingsEditor::editProperty(EditorItem* ei){
	if(ei && !ei->getSettingName().empty()){// && mCurrentSettings){
		if(!mEditView){
			mEditView = new EditView(mEngine);
			addChildPtr(mEditView);
		}
		mEditView->show();
		Settings::Setting* theSetting = &mCurrentSettings->getSetting(ei->getSettingName(), 0);
		mEditView->setSetting(theSetting, mCurrentSettings->getName());
		mEditView->setPosition(-mEditView->getScaleWidth(), 150.0f);
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
