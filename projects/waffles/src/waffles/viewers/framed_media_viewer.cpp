#include "stdafx.h"

#include "framed_media_viewer.h"
#include "waffles/waffles_events.h"
#include "waffles/model/viewer_creation_args.h"
#include "app/waffles_app_defs.h"
namespace waffles {

FramedMediaViewer::FramedMediaViewer(ds::ui::SpriteEngine& g, std::string eventChannel)
	: TitledMediaViewer(g,eventChannel) {
	
	auto tapCallback = [this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		if (mIsFullscreen) {
			hideTitle();
			hideInnerSideBar();
			mEventClient.notify(RequestViewerLaunchEvent(ViewerCreationArgs(ds::model::ContentModelRef(),
																			VIEW_TYPE_FULLSCREEN_CONTROLLER, pos,
																			ViewerCreationArgs::kViewLayerTop)));
		} else {
			showTitle();
			showInnerSideBar();
		}
	};

	auto doubleTapCallback = [this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		callAfterDelay(
			[this] {
				if (getIsFullscreen()) {
					mEventClient.notify(RequestUnFullscreenViewer(this));
				} else {
					mEventClient.notify(RequestFullscreenViewer(this));
				}
			},
			0.01f);
	};

	
	mRootLayout->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
		if (ti.mPhase == ds::ui::TouchInfo::Moved) {
			bs->passTouchToSprite(this, ti);
			return;
		}
	});
	mRootLayout->setTapCallback(tapCallback);
	mRootLayout->setDoubleTapCallback(doubleTapCallback);
	showTitle();
	showInnerSideBar();
};

void FramedMediaViewer::onLayout() {
	auto borderLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("border_layout");
	auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("inner_holdy");
	
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}

	auto nw = theLayout->getWidth();
	auto oldh = theLayout->getHeight();
	auto nh = theLayout->getWidth() / mContentAspectRatio;
	auto diff = nh - oldh;
	theLayout->setSize(nw, nh);

	if (mMediaPlayer && theLayout) {

		float w = theLayout->getWidth();
		float h = theLayout->getHeight();


		if (mMediaRotation % 2 == 0) {
			mMediaPlayer->setSize(w, h);
		} else {
			mMediaPlayer->setSize(h, w);
		}


		mMediaPlayer->setPosition(w / 2.0f, h / 2.0f);
	}

	if (mDrawingArea) {
		mDrawingArea->setSize(getWidth(), getHeight());
	}

	if (mCreationArgs.mUseHotspots && !mHotspots.empty() && mMediaPlayer) {
		if (mMediaRef.getChildren().size() != mHotspots.size()) {
			loadHotspots();
		} else {
			layoutHotspots();
		}
	}

	if (mRootLayout) {
		mRootLayout->completeAllTweens(false, true);
		mRootLayout->setSize(getWidth(), getHeight()+diff);
		mRootLayout->runLayout();
		mRootLayout->clearAnimateOnTargets(true);
	}
}

void FramedMediaViewer::onFullscreenSet() {
	TitledMediaViewer::onFullscreenSet();
	auto background = mRootLayout->getSprite<ds::ui::LayoutSprite>("player_shade");
	auto border = mRootLayout->getSprite<ds::ui::LayoutSprite>("border_layout");
	if (mIsFullscreen) {
		if (background) {
			background->hide();
		}
	} else {
		if (background) {
			background->show();
		}
	}
	mRootLayout->runLayout();
}

//void FramedMediaViewer::hideTitle() {}

//void FramedMediaViewer::hideInnerSideBar() {}


}
