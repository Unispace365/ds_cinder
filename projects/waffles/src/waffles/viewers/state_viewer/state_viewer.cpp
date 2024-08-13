#include "stdafx.h"

#include "state_viewer.h"


#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/scroll/scroll_list.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"
#include "events/state_events.h"
#include "state_detail.h"
#include "state_item.h"
#include "waffles/viewers/base_element.h"
#include "waffles/viewers/viewer_controller.h"

namespace waffles {

StateViewer::StateViewer(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mEventClient(g)
	, mRootLayout(nullptr)
	, mStateDetail(nullptr) {
	mViewerType			  = VIEW_TYPE_STATE_VIEWER;
	mMaxViewersOfThisType = 1;
	mCanResize			  = false;
	mCanArrange			  = false;


	mRootLayout = new ds::ui::SmartLayout(mEngine, "waffles/state/state_viewer.xml");
	addChildPtr(mRootLayout);

	auto detailHolder = mRootLayout->getSprite<ds::ui::LayoutSprite>("detail_holder");
	if (detailHolder) {
		mStateDetail = new StateDetail(mEngine);
		detailHolder->addChildPtr(mStateDetail);
		mStateDetail->hide();
	}

	mRootLayout->setSpriteClickFn("new_button.the_button", [this] {
		int	 theCount = 0;
		auto vc		  = ViewerController::getInstance();
		for (auto it : vc->getViewers()) {
			if (it->getViewerType() != VIEW_TYPE_TITLED_MEDIA_VIEWER) continue;
			theCount++;
		}

		ds::model::ContentModelRef newStateInfo;
		newStateInfo.setProperty("name", std::string("New State"));
		newStateInfo.setProperty("viewer_count", theCount);
		showDetails(newStateInfo);
	});

	mRootLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});

	auto fileList = mRootLayout->getSprite<ds::ui::ScrollList>("scroll_list");

	if (!fileList) {
		DS_LOG_WARNING("Didn't load a critical sprite in launcher pane!");
		return;
	}

	fileList->setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent) {
		StateItem* rpi = dynamic_cast<StateItem*>(bs);
		if (rpi && mStateDetail) {
			showDetails(rpi->getContentModel());
		}
	});

	fileList->setCreateItemCallback([this, fileList]() -> ds::ui::Sprite* {
		auto si = new StateItem(mEngine);
		si->setSize(fileList->getWidth(), si->getHeight());
		return si;
	});

	fileList->setDataCallback([this](ds::ui::Sprite* bs, int dbId) {
		StateItem* rpi = dynamic_cast<StateItem*>(bs);
		if (rpi) {
			rpi->setContentModel(mInfoMap[dbId]);
		}
	});

	fileList->setAnimateOnCallback([this](ds::ui::Sprite* bs, const float delay) {
		StateItem* rpi = dynamic_cast<StateItem*>(bs);
		if (rpi) {
			rpi->animateOn(delay);
		}
	});

	fileList->setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted) {
		StateItem* rpi = dynamic_cast<StateItem*>(bs);
		if (rpi) {
			rpi->setState(highlighted);
		}
	});


	auto scrollBar = mRootLayout->getSprite<ds::ui::ScrollBar>("scroll_bar");
	if (scrollBar) {
		scrollBar->linkScrollList(fileList);
		ci::ColorA nubbinColor = mEngine.getColors().getColorFromName("ui_highlight");
		scrollBar->getNubSprite()->setColor(nubbinColor);
		scrollBar->getBackgroundSprite()->setOpacity(0.05f);
	}


	mRootLayout->runLayout();

	const float startWidth	= mRootLayout->getWidth();
	const float startHeight = mRootLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

	mEventClient.listenToEvents<StatesUpdatedEvent>([this](auto& e) { setData(); });

	setData();
	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void StateViewer::onLayout() {
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}

	if (auto fileList = mRootLayout->getSprite<ds::ui::ScrollList>("scroll_list")) {
		fileList->forEachLoadedSprite([fileList](ds::ui::Sprite* sp) {
			if (auto item = dynamic_cast<waffles::StateItem*>(sp)) {
				item->setSize(fileList->getWidth(), sp->getHeight());
				item->layout();
			}
		});
	}
}

void StateViewer::setData() {
	mInfoMap.clear();

	auto theMedia = mEngine.mContent.getChildByName("states").getChildren();

	std::sort(theMedia.begin(), theMedia.end(), [this](ds::model::ContentModelRef a, ds::model::ContentModelRef b) {
		return a.getPropertyString("save_date") > b.getPropertyString("save_date");
	});

	std::vector<int> productIds;
	int				 mediaId = 1;
	for (auto it = theMedia.begin(); it < theMedia.end(); ++it) {
		int thisId = mediaId++;
		productIds.push_back(thisId);
		mInfoMap[thisId] = (*it);
	}

	if (auto fileList = mRootLayout->getSprite<ds::ui::ScrollList>("scroll_list")) {
		fileList->setContent(productIds);
	}

	tweenNormalized(
		0.6f, 0.0f, ci::easeNone, [this] { layout(); }, [this] { layout(); });

	layout();
}

void StateViewer::showDetails(ds::model::ContentModelRef info) {
	if (mStateDetail) {
		mStateDetail->setContentModel(info);
		mStateDetail->show();
		mStateDetail->tweenAnimateOn(true, 0.0f, 0.05f);
		mStateDetail->tweenOpacity(1.0f, mEngine.getAnimDur());
	}
}

} // namespace waffles
