#include "stdafx.h"

#include "app/waffles_app_defs.h"
#include "framed_media_viewer.h"
#include "waffles/common/ui_utils.h"
#include "waffles/model/viewer_creation_args.h"
#include "waffles/waffles_events.h"
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/media_interface_builder.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/soft_keyboard/soft_keyboard_builder.h>
#include <ds/ui/sprite/image.h>
namespace waffles {

FramedMediaViewer::FramedMediaViewer(ds::ui::SpriteEngine& g, std::string eventChannel,const std::string layoutPath)
	: TitledMediaViewer(g,eventChannel,layoutPath) {
	
	auto tapCallback = [this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		if (mIsFullscreen) {
			hideTitle();
			hideInnerSideBar();
			mEventClient.notify(RequestToggleCollapseAndMoveFullscreenController(false));
		} else if (mIsDetached) {
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

	mEventClient.listenToEvents<RequestFullscreenViewer>([this](const RequestFullscreenViewer& e) {
		if (e.mViewer != this && getIsFullscreen()) {
			mEventClient.notify(RequestUnFullscreenViewer(this));
		}
	});
	
	setTapCallback(tapCallback);
	setDoubleTapCallback(doubleTapCallback);

	showTitle();
	showInnerSideBar();
};

void FramedMediaViewer::onLayout() {
	float diff = 0;

	if (mRootLayout) {
		auto w = getWidth();
		auto h = getHeight();
		mRootLayout->setSize(w, h);
		mRootLayout->runLayout();
	}

	auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("inner_holdy");
	if (theLayout) {
		auto nw	  = theLayout->getWidth();
		auto oldh = theLayout->getHeight();
		auto nh	  = theLayout->getWidth() / mContentAspectRatio;
		diff	  = nh - oldh;

		theLayout->setSize(nw, nh);
	}

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
		/* auto nameSp = mRootLayout->getSprite<ds::ui::Text>("name");
		if (nameSp) {
			nameSp->setSize(mMediaPlayer->getWidth(), 300);
			nameSp->setResizeLimit(mMediaPlayer->getWidth(), -2);
			auto txt = nameSp->getText();
			nameSp->setText(txt);
		}*/
		mRootLayout->runLayout();
		mRootLayout->clearAnimateOnTargets(true);
	}

	// Calculate padding based on the media.
	auto root	= mRootLayout->getSprite("root_layout");
	auto player = mRootLayout->getSprite("media_player");
	if (root && player) {
		// Convert media player local coordinates to root layout coordinates.
		// Note: calling getInverseGlobalTransform() does not actually update the inverse global transform matrix!
		const auto transform   = glm::inverse(root->getGlobalTransform()) * player->getGlobalTransform();
		const auto upperLeft   = transform * ci::vec4(0, 0, 0, 1);
		const auto bottomRight = transform * ci::vec4(player->getSize(), 1);
		// Calculate padding.
		mLeftPad   = upperLeft.x / upperLeft.w;
		mRightPad  = root->getWidth() - bottomRight.x / bottomRight.w;
		mTopPad	   = upperLeft.y / upperLeft.w;
		mBottomPad = root->getHeight() - bottomRight.y / bottomRight.w;
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
		if (mMediaInterface) {
			mMediaInterface->tweenOpacity(0, 0.25, 0, ci::easeNone, [this]() { mMediaInterface->hide(); });
		}
	} else {
		if (background && mIsDetached) {
			background->show();
		}
		if (mMediaInterface) {
			mMediaInterface->show();
			mMediaInterface->tweenOpacity(1.0f, 0.25);
		}
	}
	auto mediaInterface = mMediaPlayer->getMediaInterface();
	if (mediaInterface) {
		mediaInterface->hide();
		mediaInterface->setAllowDisplay(false);
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

void FramedMediaViewer::onMediaSet() {
	TitledMediaViewer::onMediaSet();

	if (mMediaInterface) {
		mMediaInterface->release();
		mMediaInterface = nullptr;
	}


	auto interfaceHolder = mRootLayout->getSprite("controller_holder");
	auto mediaPlayer	 = getMediaPlayer();
	if (mShowingWeb) {
		auto webBacking = mRootLayout->getSprite("web_backing");
		if (webBacking) {
			webBacking->show();
		}
		if (auto filly = mRootLayout->getSprite("bg_filler")) {
			auto borderOpacity = mEngine.getWafflesSettings().getFloat("w2:viewer:border:opacity", 0, 0.8);
			filly->setOpacity(borderOpacity);
		}
	}
	
	if (mediaPlayer) {
		auto mps = mediaPlayer->getSettings();
		mps.mCanDisplayInterface = false;
		mediaPlayer->setSettings(mps);
		auto mediaInterface = mediaPlayer->getMediaInterface();
		if (mediaInterface) {
			mediaInterface->setAllowDisplay(false);
		}
	}
	if (interfaceHolder) {
		interfaceHolder->show();
		auto contentRef	 = getMedia();
		
		
		
		if (mediaPlayer && mediaPlayer->getPlayer()) {
			mMediaInterface =
				ds::ui::MediaInterfaceBuilder::buildMediaInterface(mEngine, mediaPlayer->getPlayer(), interfaceHolder);

			auto wafflesHelper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper>();
			if (wafflesHelper) {
				wafflesHelper->setMediaInterfaceStyle(mMediaInterface);
			}
			//ContentUtils::setMediaInterfaceStyle(mMediaInterface);

			if (mMediaInterface) {
				mMediaInterface->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;

				if (auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaInterface)) {
					//webInterface->setKeyboardKeyScale(30.0f / 64.0f);
					webInterface->setKeyboardDisablesTimeout(false);
					webInterface->setKeyboardAbove(false);
					webInterface->setKeyboardOnTop(true);
					ds::ui::ImageButton* keyboardBtn = webInterface->getKeyboardButton();
					webInterface->setKeyboardStateCallback([this, webInterface, keyboardBtn, wafflesHelper](const bool onScreen) {
						if (onScreen) {
							auto	  keeb		= webInterface->getSoftKeyboard();
							ci::ColorA keyb		 = mEngine.getColors().getColorFromName("viewer_background");
							wafflesHelper->setKeyboardStyle(keeb);

							auto keyboardArea = webInterface->getKeyboardArea();
							if (keyboardArea) {
								keyboardArea->enable(true);
								keyboardArea->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
								keyboardArea->setColor(keyb);
							}

							auto normalColor = mEngine.getColors().getColorFromName("ui_normal");
							auto highColor	 = mEngine.getColors().getColorFromName("ui_selected");

							webInterface->getKeyboardButton()->setNormalImageColor(highColor);
							webInterface->getKeyboardButton()->setHighImageColor(normalColor);

						} else if (!onScreen) {
							auto normalColor = mEngine.getColors().getColorFromName("ui_normal");
							auto highColor	 = mEngine.getColors().getColorFromName("ui_selected");

							webInterface->getKeyboardButton()->setNormalImageColor(normalColor);
							webInterface->getKeyboardButton()->setHighImageColor(highColor);
						}
					});
				}

				mMediaInterface->setCanTimeout(false);

				// Handle lock state changes
				//mMediaInterface->setLockStateCallback([this](bool lock) { updateLockedState(); });

				// Make sure we have the correct lock state right away too
				//updateLockedState();
			}
		}

		//setDrawingToolsState();

	} else {
		//removeDrawingTools();
	}
	//mRootLayout->runLayout();
	layout();
}

void FramedMediaViewer::onDetachedSet() {
	TitledMediaViewer::onDetachedSet();
	auto background = mRootLayout->getSprite<ds::ui::LayoutSprite>("player_shade");
	auto border		= mRootLayout->getSprite<ds::ui::LayoutSprite>("border_layout");
	if (!mIsDetached) {
		if (background) {
			background->hide();
		}
		
		auto mediaInterface = mMediaPlayer->getMediaInterface();
		if (mediaInterface) {
			mediaInterface->hide();
			mediaInterface->setAllowDisplay(false);
		}
	} else {
		if (background) {
			background->show();
		}
		
		auto mediaInterface = mMediaPlayer->getMediaInterface();
		if (mediaInterface) {
			mediaInterface->hide();
			mediaInterface->setAllowDisplay(false);
		}
	}
	
	mRootLayout->runLayout();
		
}

void FramedMediaViewer::setToFullscreen(const bool immediate, const bool showController) {

	setUnfullscreenRect(
		ci::Rectf(mPosition.x, mPosition.y, getWidth() + mPosition.x - (mRightPad+mLeftPad), getHeight() + mPosition.y - (mBottomPad+mTopPad)));
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
