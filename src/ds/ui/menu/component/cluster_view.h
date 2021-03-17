#pragma once
#ifndef DS_UI_BACKGROUND_COMPONENT_CLUSTER_VIEW
#define DS_UI_BACKGROUND_COMPONENT_CLUSTER_VIEW

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/image.h>
#include <ds/touch/five_finger_cluster.h>
#include "ds/ui/menu/touch_menu.h"

namespace ds{
namespace ui{

class MenuItem;

class ClusterView : public ds::ui::Sprite {
public:
	ClusterView(ds::ui::SpriteEngine& enginey, ds::ui::TouchMenu::TouchMenuConfig menuConfig, std::vector<ds::ui::TouchMenu::MenuItemModel>	itemModels);

	bool											getActive();

	void											updateCluster(const ds::ui::TouchInfo::Phase btp, const ds::ui::FiveFingerCluster::Cluster& cluster);

	void											startTappableMode(const ci::vec3& globalLocation, const float timeoutTime);

	/// animates this cluster off and prepares it for reuse. Called internally when there are no more fingers associated with this cluster
	void											deactivate();

private:

	// there are 2 primary states: active and invalid;
	// a cluster view is active if it has a current cluster associated with it.
	// a cluster view is invalid if the current cluster has grown too big or moved too far from it's 4-finger start point.
	// A cluster deactivates when all associated fingers are removed from the wall.
	void											activate();
	void											invalidate();

	void											buildMenuItems();
	void											activateMenu();

	void											itemActivated(MenuItem* mi);

	bool											validateCluster(const ds::ui::FiveFingerCluster::Cluster& cluster, const bool calcDist);
	void											setHighlight(ci::vec2 clusterCenter);

	void											handleInvalidateComplete();
	void											cancelTappableMode();

	void											pulseBackground(const bool bigger);

	std::vector<int>								mLineIds;
	std::vector<int>								mContainerLineIds;

	std::vector<MenuItem*>							mMenuItems;
	ds::ui::Image*									mBackground;
	ds::ui::Sprite*									mGraphicHolder;

	bool											mActive;
	bool											mInvalid;
	bool											mTappableMode;

	ds::ui::TouchMenu::TouchMenuConfig				mMenuConfig;
	std::vector<ds::ui::TouchMenu::MenuItemModel>	mItemModels;
};
} // namespace ui
} // namespace ds

#endif