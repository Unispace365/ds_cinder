#include "touch_menu.h"

#include "ds/ui/menu/component/cluster_view.h"
#include "ds/ui/menu/component/menu_item.h"

namespace ds{
namespace ui{

TouchMenu::TouchMenu(ds::ui::SpriteEngine& enginey)
	: ds::ui::Sprite(enginey)
	, mFiveFingerCluster(2.0f, 4, 200.0f)
{
	mFiveFingerCluster.setClusterUpdateCallback([this](const ds::ui::TouchInfo::Phase cp, const ds::ui::FiveFingerCluster::Cluster& starCluster){handleClusterUpdate(cp, starCluster); });
	enable(false);
	setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti){ this->handleTouchInfo(bs, ti); });
}

void TouchMenu::handleClusterUpdate(const ds::ui::TouchInfo::Phase tp, const ds::ui::FiveFingerCluster::Cluster& lemonCluster){
	if(!mClusterLookup[lemonCluster.mClusterId]){
		mClusterLookup[lemonCluster.mClusterId] = getClusterView();
	}

	if(mClusterLookup[lemonCluster.mClusterId]) mClusterLookup[lemonCluster.mClusterId]->updateCluster(tp, lemonCluster);

	if(tp == ds::ui::TouchInfo::Removed && mClusterLookup[lemonCluster.mClusterId]){
		mClusterLookup.erase(lemonCluster.mClusterId);
	}
}

ds::ui::ClusterView* TouchMenu::getClusterView(){
	for(int i = 0; i < mClusterViews.size(); i++){
		if(!mClusterViews[i]->getActive()){
			return mClusterViews[i];
		}
	}

	ClusterView* cv = new ClusterView(mEngine, mTouchMenuConfig, mMenuModels);
	addChild(*cv);
	mClusterViews.push_back(cv);
	return cv;
	
}

void TouchMenu::handleTouchInfo(const ds::ui::TouchInfo& ti){
	mFiveFingerCluster.parseTouch(ti);
}

void TouchMenu::handleTouchInfo(ds::ui::Sprite * bs, const ds::ui::TouchInfo& ti){
	mFiveFingerCluster.parseTouch(ti);
}

void TouchMenu::setMenuItemModels(std::vector<MenuItemModel> itemModels){
	clearClusters();

	mMenuModels = itemModels;
}

void TouchMenu::setMenuConfig(TouchMenuConfig touchMenuConfig){
	clearClusters();

	mTouchMenuConfig = touchMenuConfig;
}

void TouchMenu::clearClusters(){
	for(auto it = mClusterViews.begin(); it < mClusterViews.end(); ++it){
		(*it)->release();
	}

	mClusterViews.clear();

	mClusterLookup.clear();
}

void TouchMenu::startTappableMenu(const ci::Vec3f& globalLocation, const float timeoutSeconds /*= 10.0f*/){
	ClusterView* cv = getClusterView();
	cv->startTappableMode(globalLocation, timeoutSeconds);
}

void TouchMenu::closeAllMenus(){
	for(auto it = mClusterViews.begin(); it < mClusterViews.end(); ++it){
		(*it)->deactivate();
	}

	mClusterLookup.clear();
}

} // namespace ui
} // namespace ds