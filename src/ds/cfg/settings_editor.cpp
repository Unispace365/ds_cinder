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
	, mSaveApp(nullptr)
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

	if(!mSaveApp){
		std::string saveName = "%APP%/settings/" + mCurrentSettings->getName() + ".xml";
		std::string savePath = ds::Environment::expand(saveName);
		mSaveApp = new ds::ui::Text(mEngine);
		mSaveApp->setFont("Arial Bold");
		mSaveApp->setFontSize(14.0f);
		mSaveApp->mLayoutTPad = 10.0f;
		mSaveApp->mLayoutHAlign = ds::ui::LayoutSprite::kCenter;
		mSaveApp->setText("Save in " + saveName);
		mSaveApp->setColor(ci::Color(0.8f, 0.2f, 0.2f));
		mSaveApp->enable(true);
		mSaveApp->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mSaveApp->setTapCallback([this, savePath, saveName](ds::ui::Sprite* bs, const ci::vec3& pos){
			if(mCurrentSettings){
				mCurrentSettings->writeTo(savePath);

#ifdef _WIN32
				try{
					Poco::Path thePath = Poco::Path(savePath);
					thePath.makeParent();
					ShellExecute(NULL, L"open", ds::wstr_from_utf8(thePath.toString()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
				} catch(std::exception){
					// just making sure we don't crash, this isn't a required thing
				}
#endif

				mSaveApp->setText("Saved!");
				mSaveApp->callAfterDelay([this, saveName]{
					mSaveApp->setText("Save in " + saveName);
				}, 3.0f);
			}

		});

		auto theWidth = mSaveApp->getWidth();
		mSettingsLayout->addChildPtr(mSaveApp);
	}

	if(!mSaveLocal){
		std::string saveName = "%LOCAL%/settings/%PP%/" + mCurrentSettings->getName() + ".xml";
		std::string savePath = ds::Environment::expand(saveName);
		mSaveLocal = new ds::ui::Text(mEngine);
		mSaveLocal->setFont("Arial Bold");
		mSaveLocal->setFontSize(14.0f);
		mSaveLocal->mLayoutHAlign = ds::ui::LayoutSprite::kCenter;
		mSaveLocal->setText("Save in " + saveName);
		mSaveLocal->setColor(ci::Color(0.8f, 0.2f, 0.2f));
		mSaveLocal->enable(true);
		mSaveLocal->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mSaveLocal->setTapCallback([this, savePath, saveName](ds::ui::Sprite* bs, const ci::vec3& pos){
			if(mCurrentSettings){
				mCurrentSettings->writeTo(savePath);
			}

#ifdef _WIN32
			try{
				Poco::Path thePath = Poco::Path(savePath);
				thePath.makeParent();
				ShellExecute(NULL, L"open", ds::wstr_from_utf8(thePath.toString()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
			} catch(std::exception){
				// just making sure we don't crash, this isn't a required thing
			}
#endif

			mSaveLocal->setText("Saved!");
			mSaveLocal->callAfterDelay([this, saveName]{ mSaveLocal->setText("Save in " + saveName); }, 3.0f);
		});

		auto theWidth = mSaveLocal->getWidth();
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
	if(ei && !ei->getSettingName().empty() && mCurrentSettings){
		if(!mEditView){
			mEditView = new EditView(mEngine);
			addChildPtr(mEditView);
		}
		mEditView->show();
		Settings::Setting* theSetting = &mCurrentSettings->getSetting(ei->getSettingName(), 0);
		mEditView->setSetting(theSetting, mCurrentSettings->getName());
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
