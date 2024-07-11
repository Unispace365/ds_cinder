#include "stdafx.h"

#include "launcher_pane.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"
#include "waffles/common/list_item.h"

namespace waffles {

LauncherPane::LauncherPane(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/launcher/launcher_pane.xml")
	, mFileList(nullptr) {

	mFileList = getSprite<ds::ui::ScrollList>("scroll_list");

	if (!mFileList) {
		DS_LOG_WARNING("Didn't load a critical sprite in launcher pane!");
		return;
	}

	mFileList->setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent) {
		ListItem* rpi = dynamic_cast<ListItem*>(bs);
		if (rpi) {
			if (rpi->getContentModel().getPropertyString("type") == MEDIA_TYPE_DIRECTORY_CMS ||
				rpi->getContentModel().getPropertyString("type") == MEDIA_TYPE_DIRECTORY_LOCAL) {
				if (mFolderBrowseCallback) {
					mFolderBrowseCallback(rpi->getContentModel());
				}
			} else if (rpi->getContentModel().getPropertyString("type") == MEDIA_TYPE_PINBOARD) {
				/* if (mFolderBrowseCallback) {
					mFolderBrowseCallback(rpi->getContentModel());
				} */
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(
					ViewerCreationArgs(rpi->getContentModel(), VIEW_TYPE_TITLED_MEDIA_VIEWER, cent)));
			}else {
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(
					ViewerCreationArgs(rpi->getContentModel(), VIEW_TYPE_TITLED_MEDIA_VIEWER, cent)));
			}
		}
	});

	mFileList->setCreateItemCallback([this]() -> ds::ui::Sprite* {
		auto li = new ListItem(mEngine, "waffles/common/filter_item.xml");
		li->setSize(mFileList->getWidth(), li->getHeight());
		li->setSpriteClickFn("the_arrow", [this, li] {
			if (mFolderBrowseCallback) {
				mFolderBrowseCallback(li->getContentModel());
			}
		});
		return li;
	});

	mFileList->setDataCallback([this](ds::ui::Sprite* bs, int dbId) {
		ListItem* rpi = dynamic_cast<ListItem*>(bs);
		if (rpi) {
			rpi->setContentModel(mInfoMap[dbId]);
		}
	});

	mFileList->setAnimateOnCallback([](ds::ui::Sprite* bs, const float delay) {
		ListItem* rpi = dynamic_cast<ListItem*>(bs);
		if (rpi) {
			rpi->animateOn(delay);
		}
	});

	mFileList->setStateChangeCallback([](ds::ui::Sprite* bs, const bool highlighted) {
		ListItem* rpi = dynamic_cast<ListItem*>(bs);
		if (rpi) {
			rpi->setState(highlighted);
		}
	});

	auto scrollBar = getSprite<ds::ui::ScrollBar>("scroll_bar");
	if (scrollBar) {
		scrollBar->linkScrollList(mFileList);
		ci::ColorA nubbinColor = mEngine.getColors().getColorFromName("ui_highlight");
		scrollBar->getNubSprite()->setColor(nubbinColor);
		scrollBar->getBackgroundSprite()->setOpacity(0.05f);
	}
}

void LauncherPane::setData(std::vector<ds::model::ContentModelRef> theMedia) {

	if (!mFileList) return;

	mInfoMap.clear();

	std::vector<int> productIds;
	int				 mediaId = 1;
	for (auto it : theMedia) {
		int thisId = mediaId++;
		productIds.push_back(thisId);
		mInfoMap[thisId] = it;
	}

	mFileList->setContent(productIds);
	layout();
}

void LauncherPane::animateOn() {
	tweenAnimateOn(true, 0.0f, 0.025f);
}

bool LauncherPane::getBackMediaModel(ds::model::ContentModelRef& outModel) {
	if (mInfoMap.empty()) return false;

	outModel = mEngine.mContent.getChildByName("cms_root")
				   .getDescendant("waffles_nodes", mInfoMap.begin()->second.getPropertyInt("parent_id"));

	if (!outModel.empty()) {
		return true;
	}

	return false;
}

void LauncherPane::onSizeChanged() {
	layout();
}

void LauncherPane::layout() {
	runLayout();

	if (mFileList) {

		mFileList->forEachLoadedSprite([this](ds::ui::Sprite* bs) {
			ListItem* rpi = dynamic_cast<ListItem*>(bs);
			if (rpi) {
				rpi->setSize(mFileList->getWidth(), rpi->getHeight());
				rpi->runLayout();
			}
		});
	}
}


} // namespace waffles
