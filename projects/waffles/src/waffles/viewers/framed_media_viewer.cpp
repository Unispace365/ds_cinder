#include "stdafx.h"

#include "framed_media_viewer.h"
#include "waffles/waffles_events.h"
#include "waffles/model/viewer_creation_args.h"
#include "app/waffles_app_defs.h"
namespace waffles {

FramedMediaViewer::FramedMediaViewer(ds::ui::SpriteEngine& g, std::string eventChannel,const std::string layoutPath)
	: TitledMediaViewer(g,eventChannel,layoutPath) {
	
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
	
	auto background = mRootLayout->getSprite("background");
	if (background) {
		background->setTapCallback(tapCallback);
		background->setDoubleTapCallback(doubleTapCallback);
		background->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
			if (ti.mPhase == ds::ui::TouchInfo::Moved) {
				bs->passTouchToSprite(this, ti);
				return;
			}
		});
	}

	auto border = mRootLayout->getSprite("border_layout");
	auto title	= mRootLayout->getSprite("title_layout");
	auto sidebar	= mRootLayout->getSprite("ui_holder");

	if (border && title && sidebar) {
		mLeftPad = border->mLayoutLPad;
		mRightPad = border->mLayoutRPad;
		mTopPad	  = border->mLayoutTPad + title->getHeight();
		mBottomPad = border->mLayoutBPad + sidebar->getHeight();
	}
	
	setTapCallback(tapCallback);
	setDoubleTapCallback(doubleTapCallback);
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

	auto border	 = mRootLayout->getSprite("border_layout");
	auto title	 = mRootLayout->getSprite("title_layout");
	auto sidebar = mRootLayout->getSprite("ui_holder");
	if (border && title && sidebar) {
		mLeftPad   = border->mLayoutLPad;
		mRightPad  = border->mLayoutRPad;
		mTopPad	   = border->mLayoutTPad + title->getHeight();
		mBottomPad = border->mLayoutBPad*2.0 + sidebar->getHeight();
	}

	//auto frameCenter   = mRootLayout->getGlobalCenterPosition();
	//auto contentCenter = theLayout->getGlobalCenterPosition();
	//auto offset		   = (contentCenter.y - frameCenter.y) / mRootLayout->getHeight();
	//mRootLayout->setCenter(0.5, 0.5 + offset);
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

void FramedMediaViewer::showTitle() {
	if (!mRootLayout || mShowingTitle || mShowingKeyboard) return;
	mShowingTitle = true;
	if (auto titleHodler = mRootLayout->getSprite("title_layout")) {
		titleHodler->show();
		titleHodler->tweenOpacity(1.0f, mEngine.getAnimDur());
	}
	onLayout();

}

void FramedMediaViewer::setToFullscreen(const bool immediate, const bool showController) {

	auto		normalLayer	 = ViewerControllerFactory::getInstanceOf(ci::vec2(), getChannelName())->getNormalLayer();
	const float screenWidth	 = normalLayer->getWidth();	 // mDisplaySize.x;
	const float screenHeight = normalLayer->getHeight(); // mDisplaySize.y;
	const float screenAsp	 = screenWidth / screenHeight;

	bool didWebSpecial = false;
	
	if (auto mp = getMediaPlayer()) {
		if (auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mp->getPlayer())) {
			mp->setWebViewSize(ci::vec2(screenWidth, screenHeight));
			mp->setSize(ci::vec2(screenWidth, screenHeight));
			mContentAspectRatio = screenAsp;
			if (immediate) {
				setViewerWidth(screenWidth);
				setPosition(0.0f, 0.0f);
			} else {
				animateWidthTo(screenWidth);
				tweenPosition(ci::vec3(0.0f), getAnimateDuration(), 0.0f, ci::easeInOutQuad);
			}
			setIsFullscreen(true);
			didWebSpecial = true;
		}
	}
	if (!didWebSpecial) {
		auto  playerHolder = mRootLayout->getSprite<ds::ui::LayoutSprite>("player_hodler");
		
		float viewerAsp	   = mMediaPlayer->getWidth() / mMediaPlayer->getHeight();
		float viewerScale = getScale().x;
		auto  xxtra		   = (getWidth()  - mMediaPlayer->getWidth())/viewerScale;
		auto  yxtra		   = (getHeight() - mMediaPlayer->getHeight())/viewerScale;
		
		if (viewerScale == 0.0f) viewerScale = 0.001f;
		if (viewerAsp > screenAsp) {
			auto width	= screenWidth / viewerScale;
			auto height = screenWidth / viewerAsp;
			auto x		= 0;
			auto y		= screenHeight * 0.5 - height * 0.5;
			if (immediate) {
				setViewerWidth(width);
				setPosition(x, y);
			} else {
				animateWidthTo(width);
				tweenPosition(ci::vec3(x, y, 0.0f), getAnimateDuration(), 0.0f, ci::easeInOutQuad);
			}
		} else {
			auto height = screenHeight / viewerScale - yxtra;
			auto width	= screenHeight * viewerAsp - xxtra;
			auto y		= 0;
			auto x		= screenWidth * 0.5 - width * 0.5;
			if (immediate) {
				setViewerHeight(height);
				setPosition(x, y);
			} else {
				animateHeightTo(height);
				tweenPosition(ci::vec3(x, y, 0.0f), getAnimateDuration(), 0.0f, ci::easeInOutQuad);
			}
		}
	}
}



//void FramedMediaViewer::hideTitle() {}

//void FramedMediaViewer::hideInnerSideBar() {}


}
