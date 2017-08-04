#include "stdafx.h"

#include "settings_editor.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/cfg/editor_components/editor_item.h"

namespace ds{
namespace cfg{

SettingsEditor::SettingsEditor(ds::ui::SpriteEngine& e)
	: ds::ui::Sprite(e)
	, mCurrentSettings(nullptr)
	, mPrimaryLayout(nullptr)
	, mSettingsLayout(nullptr)
{
	mPrimaryLayout = new ds::ui::LayoutSprite(mEngine);
	addChildPtr(mPrimaryLayout);
	mPrimaryLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mPrimaryLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mPrimaryLayout->setClipping(true);


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

	mSettingItems.clear();

	if(!mSettingsLayout || !mPrimaryLayout) return;

	theSettings->forEachSetting([this](ds::cfg::Settings::Setting& setting){
		EditorItem* ei = new EditorItem(mEngine);
		ei->setSetting(&setting);
		if(mSettingsLayout){
			mSettingsLayout->addChildPtr(ei);
		}
	});

	mPrimaryLayout->runLayout();

	setPosition(mEngine.getSrcRect().getX2() - mPrimaryLayout->getWidth(), mEngine.getSrcRect().getY1());

}

}
}
