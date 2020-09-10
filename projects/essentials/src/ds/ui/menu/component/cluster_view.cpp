#include "stdafx.h"

#include "cluster_view.h"

#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>
#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/ui/tween/tweenline.h>
#include <cinder/CinderMath.h>
#include "ds/ui/menu/component/menu_item.h"

namespace ds{
namespace ui{

ClusterView::ClusterView(ds::ui::SpriteEngine& enginey, ds::ui::TouchMenu::TouchMenuConfig menuConfig, std::vector<ds::ui::TouchMenu::MenuItemModel> itemModels)
	: ds::ui::Sprite(enginey)
	, mActive(false)
	, mInvalid(false)
	, mBackground(nullptr)
	, mMenuConfig(menuConfig)
	, mItemModels(itemModels)
	, mTappableMode(false)
	, mGraphicHolder(nullptr)
{

	mBackground = new ds::ui::Image(mEngine);
	if(mBackground){
		addChild(*mBackground);
		mBackground->setCenter(0.5f, 0.5f);
		mBackground->setPosition(mMenuConfig.mBackgroundOffset);
		mBackground->setColor(mMenuConfig.mBackgroundColor);
		mBackground->setScale(0.0f, 0.0f, 0.0f);
		mBackground->setOpacity(0.0f);

		if(!mMenuConfig.mBackgroundImage.empty()){
			mBackground->setImageFile(mMenuConfig.mBackgroundImage);
		}
	}

	mGraphicHolder = new ds::ui::Sprite(mEngine);
	mGraphicHolder->setCenter(0.5f, 0.5f);
	addChildPtr(mGraphicHolder);

	enable(false);
	buildMenuItems();
}

bool ClusterView::getActive(){
	return mActive;
}

void ClusterView::buildMenuItems(){

	for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
		(*it)->release();
	}

	mMenuItems.clear();

	if(mItemModels.empty()){
		return;
	}

	float mis = (float)mItemModels.size();
	int i = 0;
	ci::vec2 newPos = ci::vec2(mMenuConfig.mClusterRadius / 2.0f, -mMenuConfig.mItemSize.y * (mis) / 2.0f);
	for(auto it = mItemModels.begin(); it < mItemModels.end(); ++it){
		MenuItem* mi = new MenuItem(mEngine, (*it), mMenuConfig);
		addChildPtr(mi);
		mMenuItems.push_back(mi);

		float			degs = mMenuConfig.mClusterPositionOffset + ((float)(i)* 360.0f / (mis));
		while(degs >= 360.0f) degs -= 360.0f;

		const float		radians = -ci::toRadians(degs);
		newPos = ci::vec2(mMenuConfig.mClusterRadius * cos(radians), mMenuConfig.mClusterRadius * sin(radians));
		newPos.x -= mi->getWidth() / 2.0f;
		newPos.y -= mi->getHeight() / 2.0f;

		mi->setAngle(degs);
		mi->setPosition(newPos.x, newPos.y);
		i++;
	}

}

void ClusterView::startTappableMode(const ci::vec3& globalLocation, const float timeoutTime){
	mTappableMode = true;
	mInvalid = false;
	setPosition(globalLocation);
	callAfterDelay([this]{cancelTappableMode(); }, timeoutTime);
	activate();

	for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
		MenuItem* mi = (*it);
		mi->enable(true);
		mi->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);

		mi->setProcessTouchCallback([this, mi](ds::ui::Sprite*, const ds::ui::TouchInfo& ti){
			if(ti.mNumberFingers == 1 && ti.mPhase == ds::ui::TouchInfo::Added){
				mi->highlight();
			}

			
			if(ti.mPhase == ds::ui::TouchInfo::Moved && ci::distance( ti.mCurrentGlobalPoint,ti.mStartPoint) > mEngine.getMinTapDistance()){
				mi->unhighlight();
			}
		});

		mi->setTapCallback([this, mi](ds::ui::Sprite*, const ci::vec3& pos){
			if(mi && mi->getModel().mActivatedCallback){
				mi->getModel().mActivatedCallback(pos);
				cancelTappableMode();
			}
		});
	}

	if(mBackground){
		mBackground->enable(true);
		mBackground->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		mBackground->setTapCallback([this](ds::ui::Sprite*, const ci::vec3& pos){
			cancelTappableMode();
		});
	}
}

void ClusterView::cancelTappableMode(){
	mTappableMode = false;

	for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
		MenuItem* mi = (*it);
		mi->enable(false);
	}
	if(mBackground){
		mBackground->enable(false);
	}

	deactivate();

}

void ClusterView::activateMenu() {

	DS_LOG_VERBOSE(2, "ClusterView: activateMenu() at pos=" << getGlobalPosition());

	for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
		(*it)->animateOn();
	}

	if(mMenuConfig.mActivatedCallback){
		mMenuConfig.mActivatedCallback(this, mGraphicHolder);
	}
	
}

void ClusterView::setHighlight(ci::vec2 clusterCenter) {

	DS_LOG_VERBOSE(4, "ClusterView: setHighlight() at pos=" << clusterCenter);

	bool somethingHighlighted(false);
	for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
		if((*it)->contains(ci::vec3(clusterCenter.x, clusterCenter.y, 0.0f)) && !somethingHighlighted){
			(*it)->highlight();

			somethingHighlighted = true;

		} else {
			(*it)->unhighlight();
		}
	}
}


void ClusterView::updateCluster(const ds::ui::TouchInfo::Phase btp, const ds::ui::FiveFingerCluster::Cluster& cluster){
	if(!mActive
	   && cluster.mTouches.size() > 4
	   && validateCluster(cluster, false)){
		activate();
		setPosition(cluster.mCurrentBoundingBox.getCenter().x, cluster.mCurrentBoundingBox.getCenter().y);
	}

	if(mActive && !mInvalid){

		// Make sure we've grabbed all the touches and they aren't doing stuff to other sprites.
		for(auto it = cluster.mTouches.begin(); it < cluster.mTouches.end(); ++it){
			ds::ui::Sprite* target = mEngine.getSpriteForFinger((*it).mFingerId);
			// If there's no target, then don't worry about it, since this sprite doesn't need to own the touch
			if(target && target != this){
				target->passTouchToSprite(this, (*it));
			}
		}

		ci::vec2 clusterCenter = cluster.mCurrentBoundingBox.getCenter();
		setHighlight(clusterCenter);

		if(!validateCluster(cluster, true)){
			invalidate();
		}
	}

	if(btp == ds::ui::TouchInfo::Removed && !mMenuItems.empty()){
	//	if(!mInvalid){
			for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
				if((*it)->getHighlited()){
					itemActivated((*it));
					break;
				}
			}
		//}
		deactivate();
	}
}

void ClusterView::itemActivated(MenuItem* mi) {

	ci::vec3 launchPos = this->localToGlobal(mi->getCenter());

	if(mi && mi->getModel().mActivatedCallback) {
		DS_LOG_VERBOSEW(2, L"ClusterView: itemActivated() " << mi->getModel().mTitle);
		mi->getModel().mActivatedCallback(launchPos);
	}
}

bool ClusterView::validateCluster(const ds::ui::FiveFingerCluster::Cluster& cluster, const bool calcDist){

	float sizeThreshold = mMenuConfig.mClusterSizeThreshold;
	float distThreshold = mMenuConfig.mClusterDistanceThreshold;

	if(cluster.mCurrentBoundingBox.getWidth() > sizeThreshold || cluster.mCurrentBoundingBox.getHeight() > sizeThreshold){
		return false;
	}

	if(calcDist){
		ci::vec2 bbcent = cluster.mCurrentBoundingBox.getCenter();
		ci::vec3 center = getPosition();
		float xdelt = bbcent.x - center.x;
		float ydelt = bbcent.y - center.y;
		if(distThreshold * distThreshold < (xdelt * xdelt) + (ydelt * ydelt)){
			return false;
		}
	}

	return true;
}

void ClusterView::activate(){
	show();
	activateMenu();

	mActive = true;

	if(mBackground){
		float bgOpacity = mMenuConfig.mBackgroundOpacity;
		float bgScale = mMenuConfig.mBackgroundScale;
		mBackground->animStop();
		mBackground->tweenScale(ci::vec3(bgScale, bgScale, 1.0f), mMenuConfig.mAnimationDuration, 0.0f, ci::easeInOutCubic, [this] { pulseBackground(true); });
		mBackground->tweenOpacity(bgOpacity, mMenuConfig.mAnimationDuration);
	}
}

void ClusterView::pulseBackground(const bool bigger) {

	float destScale = mMenuConfig.mBackgroundScale;
	if (bigger) destScale = destScale * mMenuConfig.mBackgroundPulseAmount;

	mBackground->tweenScale(ci::vec3(destScale, destScale, 1.0f), mMenuConfig.mAnimationDuration * 2.0f, 0.0f, ci::easeOutCubic, [this, bigger] { pulseBackground(!bigger); });
}

void ClusterView::deactivate(){
	invalidate();

	mInvalid = false;
	mActive = false;

	if(mMenuConfig.mDeactivatedCallback){
		mMenuConfig.mDeactivatedCallback(this, mGraphicHolder);
	}
}

void ClusterView::invalidate(){
	if(!mInvalid){

		for(auto it = mMenuItems.begin(); it < mMenuItems.end(); ++it){
			(*it)->unhighlight();
			(*it)->animateOff();
		}

		if(mBackground){
			mBackground->animStop();
			mBackground->tweenScale(ci::vec3(), mMenuConfig.mAnimationDuration, 0.0f, ci::easeInCubic, [this]{handleInvalidateComplete(); });
			mBackground->tweenOpacity(0.0f, mMenuConfig.mAnimationDuration);
		}
	}

	mInvalid = true;
}

void ClusterView::handleInvalidateComplete(){
	hide();
}


} // namespace ui
} // namespace ds