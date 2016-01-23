#include "menu_item.h"

#include <boost/algorithm/string.hpp>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/ui/tween/tweenline.h>
#include <ds/util/string_util.h>

namespace ds{
namespace ui{

MenuItem::MenuItem(ds::ui::SpriteEngine& enginey, const ds::ui::TouchMenu::MenuItemModel itemModel, const ds::ui::TouchMenu::TouchMenuConfig config)
	: ds::ui::Sprite(enginey)
	, mClippy(nullptr)
	, mMenuItemModel(itemModel)
	, mMenuConfig(config)
	, mHighlighted(false)
	, mRotation(0.0f)
	, mClipper(nullptr)
	, mIcon(nullptr)
	, mIconGlow(nullptr)
	, mIconsMatch(false)
	, mActive(false)
	, mLines(nullptr)
	, mTitle(nullptr)
	, mSubtitle(nullptr)
{

	if(mMenuItemModel.mIconHighlightedImage.empty()){
		mMenuItemModel.mIconHighlightedImage = mMenuItemModel.mIconNormalImage;
	}

	if(!mMenuItemModel.mIconNormalImage.empty()){
		mIcon = new ds::ui::Image(mEngine, mMenuItemModel.mIconNormalImage);
	}

	if(!mMenuItemModel.mIconHighlightedImage.empty()){
		mIconGlow = new ds::ui::Image(mEngine, mMenuItemModel.mIconHighlightedImage);
	}

	if(mIcon && mIconGlow){
		mIconsMatch = (mMenuItemModel.mIconNormalImage == mMenuItemModel.mIconHighlightedImage);
	}

	float iconHeight = mMenuConfig.mItemIconHeight;
	float padding = mMenuConfig.mItemTitlePad; 
	float titleYPercent = mMenuConfig.mItemTitleYPositionPercent;
	ci::Vec2f thisSize = mMenuConfig.mItemSize;
	setSize(thisSize.x, thisSize.y);

	mClipper = new ds::ui::Sprite(mEngine, getWidth(), getHeight());
	mClipper->enable(false);
	mClipper->setClipping(true);
	addChild(*mClipper);

	mClippy = new ds::ui::Sprite(mEngine);
	mClippy->enable(false);
	mClipper->addChild(*mClippy);

	if(!mMenuConfig.mItemTitleTextConfig.empty()){
		mTitle = mEngine.getEngineCfg().getText(mMenuConfig.mItemTitleTextConfig).create(mEngine, this);
	}

	if(!mMenuConfig.mItemSubtitleTextConfig.empty()){
		mSubtitle = mEngine.getEngineCfg().getText(mMenuConfig.mItemSubtitleTextConfig).create(mEngine, this);
	}

	float titlePositiony = thisSize.y * titleYPercent;
	float subtitlePositiony = titlePositiony;
	if(mTitle){
		mTitle->setText(mMenuItemModel.mTitle);
		ci::Vec2f titleSize = ci::Vec2f(mTitle->getWidth(), mTitle->getHeight());
		mTitle->setPosition(thisSize.x * 0.5f - titleSize.x * 0.5f, titlePositiony);
		mTitle->setOpacity(0.5f);
		mClippy->addChildPtr(mTitle);
		subtitlePositiony += (titleSize.y * 1.1f);
	}

	if(mSubtitle){
		mSubtitle->setText(mMenuItemModel.mSubtitle);
		ci::Vec2f titleSize = ci::Vec2f(mSubtitle->getWidth(), mSubtitle->getHeight());
		mSubtitle->setPosition(thisSize.x * 0.5f - titleSize.x * 0.5f, subtitlePositiony);
		mSubtitle->setOpacity(0.5f);
		mClippy->addChildPtr(mSubtitle);
	}

	mClippy->setPosition(0.0f, -getHeight());

	if(mIcon){
		float newScale = iconHeight / mIcon->getHeight();
		mClippy->addChild(*mIcon);
		mIcon->setCenter(0.5f, 0.5f);
		mIcon->setScale(newScale, newScale);
		mIcon->setPosition(getWidth() * 0.5f, titlePositiony - (iconHeight * 0.5f * newScale) - padding);
	}
	if(mIcon && mIconGlow) {
		mClippy->addChild(*mIconGlow);
		mIconGlow->setCenter(mIcon->getCenter());
		mIconGlow->setScale(mIcon->getScale());
		mIconGlow->setPosition(mIcon->getPosition());
		mIconGlow->setOpacity(0.0f);
	}

	enable(false);
}

void MenuItem::animateOn(int direction){
	if(!mClippy || mActive) return;
	if(direction == 0){
		mClippy->setPosition(-getWidth(), 0.0f);
	} else if(direction == 1) {
		mClippy->setPosition(0.0f, -getHeight());
	} else if(direction == 2){
		mClippy->setPosition(getWidth(), 0.0f);
	} else if(direction == 3){
		mClippy->setPosition(0.0f, getHeight());
	}

	mClippy->tweenPosition(ci::Vec3f::zero(), mMenuConfig.mAnimationDuration, 0.0f, ci::easeInOutCubic);

	mActive = true;
}

void MenuItem::animateOff(int direction){
	if(!mClippy || !mActive) return;


	ci::Vec3f destPos = ci::Vec3f::zero();
	if(direction == 2){
		destPos.set(-getWidth(), 0.0f, 0.0f);
	} else if(direction == 3) {
		destPos.set(0.0f, -getHeight(), 0.0f);
	} else if(direction == 0){
		destPos.set(getWidth(), 0.0f, 0.0f);
	} else if(direction == 1){
		destPos.set(0.0f, getHeight(), 0.0f);
	}

	mClippy->tweenPosition(destPos, mMenuConfig.mAnimationDuration, 0.0f, ci::easeInOutCubic);

	mActive = false;
}

void MenuItem::highlight(){
	if(!mHighlighted){
		if(mTitle){
			mTitle->tweenOpacity(1.0f, mMenuConfig.mAnimationDuration);
		}
		if(mSubtitle){
			mSubtitle->tweenOpacity(1.0f, mMenuConfig.mAnimationDuration);
		}
		if(mIcon && mIconGlow && !mIconsMatch) {
			mIcon->tweenOpacity(0.0f, mMenuConfig.mAnimationDuration);
			mIconGlow->tweenOpacity(1.0f, mMenuConfig.mAnimationDuration);
		}
	}

	mHighlighted = true;
}

void MenuItem::unhighlight(){
	if(mHighlighted){
		if(mTitle){
			mTitle->tweenOpacity(0.5f, mMenuConfig.mAnimationDuration);
		}
		if(mSubtitle){
			mSubtitle->tweenOpacity(0.5f, mMenuConfig.mAnimationDuration);
		}
		if(mIcon && mIconGlow && !mIconsMatch) {
			mIcon->tweenOpacity(1.0f, mMenuConfig.mAnimationDuration);
			mIconGlow->tweenOpacity(0.0f, mMenuConfig.mAnimationDuration);
		}
	}

	mHighlighted = false;
}

} // namespace ui
} // namespace ds