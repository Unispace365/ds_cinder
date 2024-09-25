#include "stdafx.h"

#include "titled_media_viewer.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/media/player/pdf_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/media/player/web_player.h>
#include <ds/ui/media/player/youtube_player.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/sprite/border.h>
#include <ds/ui/sprite/gst_video.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/web.h>
#include <ds/util/file_meta_data.h>


#include "app/waffles_app_defs.h"

#include "waffles/waffles_events.h"
#include "waffles/common/ui_utils.h"
#include "waffles/pinboard/pinboard_button.h"
#include "waffles/util/capture_player.h"
#include "waffles/util/shadow_layout.h"
#include "waffles/util/waffles_helper.h"


namespace waffles {

TitledMediaViewer::TitledMediaViewer(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mMediaPlayer(nullptr)
	, mDrawingMode(false)
	, mShowingOptions(false)
	, mShowingTitle(false)
	, mInitialLoadError(false)
	, mShowingVideo(false)
	, mShowingWeb(false)
	, mMediaRotation(0)
	, mPlayerLoadedTimer(mEngine)
	, mControlsTimeoutTimer(mEngine) {

	mViewerType	  = VIEW_TYPE_TITLED_MEDIA_VIEWER;
	mAnimDuration = mEngine.getAnimDur();

	const float minSize = mEngine.getWafflesSettings().getFloat("media_viewer:min_size", 0, 400.0f);
	mAbsMinSize.x		= minSize;
	mAbsMinSize.y		= minSize;

	const float maxSize = mEngine.getWafflesSettings().getFloat("media_viewer:max_size", 0, 400.0f);
	mAbsMaxSize.x		= maxSize;
	mAbsMaxSize.y		= maxSize;

	

	/* setTransparent(false);
	setColor(ci::Color(1.f, 0.f, 1.f)); */

	auto tapCallback = [this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		if (mIsFullscreen) {
			hideTitle();
			hideInnerSideBar();
			mEventClient.notify(RequestViewerLaunchEvent(
				ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_FULLSCREEN_CONTROLLER, pos,
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

	mRootLayout = new ds::ui::SmartLayout(mEngine, "waffles/viewer/titled_media_viewer.xml");
	addChildPtr(mRootLayout);
	mRootLayout->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
		if (ti.mPhase == ds::ui::TouchInfo::Moved) {
			bs->passTouchToSprite(this, ti);
			return;
		}
	});
	mRootLayout->setTapCallback(tapCallback);
	mRootLayout->setDoubleTapCallback(doubleTapCallback);

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

	setTapCallback(tapCallback);
	setDoubleTapCallback(doubleTapCallback);


	mMediaPlayer = mRootLayout->getSprite<ds::ui::MediaPlayer>("media_player");
	if (mMediaPlayer) {
		mMediaPlayer->setStatusCallback([this](const bool isAllGood) {
			const bool wasErrored = mInitialLoadError;
			mInitialLoadError	  = false;
			if (!mRootLayout) return;
			if (auto placehodler = mRootLayout->getSprite("loading_placeholder")) {
				placehodler->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
										  [placehodler] { placehodler->hide(); });
			}
			if (wasErrored) {
				if (auto theBody = mRootLayout->getSprite<ds::ui::Text>("body")) {
					theBody->hide();
					theBody->setText("");
					layout();
					hideTitle();
				}
			}
		});

		mMediaPlayer->setErrorCallback([this](const std::string& msg) {
			mInitialLoadError = true;
			if (!mRootLayout) return;
			if (auto theBody = mRootLayout->getSprite<ds::ui::Text>("body")) {
				theBody->show();
				theBody->setText(msg);
				layout();
				showTitle();
			}
		});
	}

	mRootLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});
	mRootLayout->setSpriteClickFn("drawing.the_button", [this] { toggleDrawing(); });
	mRootLayout->setSpriteClickFn("options.the_button", [this] { toggleOptions(); });
	mRootLayout->setSpriteClickFn("rotate.the_button", [this] { rotateMedia(); });
	mRootLayout->setSpriteClickFn("set_background.the_button", [this] {
		auto thePlayer = getMediaPlayer()->getPlayer();
		auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(thePlayer);
		int	 pdfPage   = 0;
		if (pdfPlayer && pdfPlayer->getPDF()) {
			pdfPage = pdfPlayer->getPDF()->getPageNum();
		}
		mEventClient.notify(RequestBackgroundChange(BACKGROUND_TYPE_USER_MEDIA, mMediaRef, pdfPage));
	});

	mRootLayout->setSpriteClickFn("particles.the_button", [this] {
		auto thePlayer = getMediaPlayer()->getPlayer();
		auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(thePlayer);
		int	 pdfPage   = 0;
		if (pdfPlayer && pdfPlayer->getPDF()) {
			pdfPage = pdfPlayer->getPDF()->getPageNum();
		}

		// First save the actual background as the user background
		mEventClient.notify(RequestBackgroundChange(BACKGROUND_TYPE_USER_MEDIA, mMediaRef, 0));

		// Then send that same background into the particles mode
		auto particleBackground = mEngine.mContent.getChildByName("background.user").getChild(0);
		if (pdfPage != 0) {
			particleBackground.setProperty("pdf_page", pdfPage);
		}
		auto mMediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(particleBackground);
		particleBackground.setPropertyResource(mMediaPropertyKey, getMedia().getPropertyResource(mMediaPropertyKey));// TODO: cannot tell if this wants mMediaPropertyKey
		mEventClient.notify(RequestBackgroundChange(BACKGROUND_TYPE_PARTICLES, ds::model::ContentModelRef()));
	});
	mRootLayout->setSpriteClickFn("duplicate.the_button", [this] {
		ViewerCreationArgs args = getDuplicateCreationArgs();
		args.mLocation.x += 50.0f;
		args.mLocation.y += 50.0f;
		mEventClient.notify(RequestViewerLaunchEvent(args));
	});

	mRootLayout->setSpriteClickFn("fullscreen.the_button", [this] {
		if (getIsFullscreen()) {
			mEventClient.notify(RequestUnFullscreenViewer(this));
		} else {
			mEventClient.notify(RequestFullscreenViewer(this));
		}
	});
}

void TitledMediaViewer::calculateSizeLimits() {
	if (!mMediaPlayer) return;

	mContentAspectRatio = mMediaPlayer->getContentAspectRatio();

	float contentWidth	= mMediaPlayer->getWidth();
	float contentHeight = mMediaPlayer->getHeight();

	if (contentWidth < 1.0f) contentWidth = 1.0f; // prevents divide-by-zero errors
	if (contentHeight < 1.0f) contentHeight = 1.0f;

	// calculate a default size that maximizes size
	float settingsAspect = 1.0f;
	float settingsWidth	 = mMediaPlayer->getSettings().mDefaultBounds.x;
	float settingsHeight = mMediaPlayer->getSettings().mDefaultBounds.y;
	/* if(mShowingWebcam){
			settingsWidth = contentWidth;
			settingsHeight = contentHeight;
	} */
	if (settingsHeight > 0.0f) {
		settingsAspect = settingsWidth / settingsHeight;
	}

	// calculate a width to make the player fit maximally
	float scaleFactor = 1.0f;
	float idealWidth  = settingsWidth;
	float idealHeight = settingsHeight;
	if (mContentAspectRatio < settingsAspect) {
		scaleFactor = settingsHeight / contentHeight;
		idealWidth	= contentWidth * scaleFactor;
	} else if (mContentAspectRatio > settingsAspect) {
		scaleFactor = settingsWidth / contentWidth;
		idealHeight = contentHeight * scaleFactor;
	}

	mDefaultSize = ci::vec2(idealWidth, idealHeight);

	setSize(mDefaultSize.x, mDefaultSize.y);
	setSizeLimits();
	setViewerSize(mDefaultSize.x, mDefaultSize.y);
}

void TitledMediaViewer::onMediaSet() {
	mInitialLoadError = false;
	if (!mMediaPlayer) {
		mInitialLoadError = true;
		mFatalError		  = true;
		return;
	}

	if (mDrawingMode) {
		toggleDrawing();
	}

	if (mDrawingArea) {
		mDrawingArea->release();
		mDrawingArea = nullptr;
	}

	if (auto pb = mRootLayout->getSprite<PinboardButton>("pinboard")) {
		pb->setContentModel(mMediaRef);
	}

	auto mMediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
	auto primaryResource = mMediaRef.getPropertyResource(mMediaPropertyKey);
	if (primaryResource.empty()) primaryResource = mMediaRef.getPropertyResource("media_media_res");

	bool gifSpecial = false;

	if (mCreationArgs.mEnforceMinSize == false) {
		mAbsMinSize = ci::vec2(20.f, 20.f);
	}

	auto mvs				  = mMediaPlayer->getSettings();
	mvs.mWebKeyboardAbove	  = false;
	mvs.mShowInterfaceAtStart = false;
	mvs.mVideoResetOnComplete = !isIdling() || (mCreationArgs.mLooped);
	mvs.mMipMapImages		  = true;
	mvs.mWebAllowTouchToggle  = true;
	mvs.mCanDisplayInterface  = true;
	mvs.mWebStartTouchable	  = mCreationArgs.mStartLocked;


	auto webSize = mEngine.getWafflesSettings().getVec2("web:default_size", 0, ci::vec2(-1.0f, -1.0f));
	if (primaryResource.getAbsoluteFilePath().find(".gif") != std::string::npos) {
		try {
			// Awkwardly load the image from the network so we know how big to make it
			auto imgy = ci::loadImage(ci::loadUrl(primaryResource.getAbsoluteFilePath()));
			primaryResource.setWidth(imgy->getWidth());
			primaryResource.setHeight(imgy->getHeight());
			mvs.mWebStartTouchable = false;
			// Pad the webSize so no scrollbars appear
			webSize = ci::vec2(primaryResource.getWidth() + 2.f, primaryResource.getHeight() + 2.f);
		} catch (const std::exception& e) {
			DS_LOG_ERROR("Failed to load gif!");
		}
		gifSpecial = true;
	}

	if (webSize.x > 0.0f && webSize.y > 0.0f) {
		mvs.mWebDefaultSize = webSize;
		mMediaPlayer->setWebViewSize(webSize);
		mvs.mDefaultBounds = webSize;
	}

	double streamLatency	   = (double)mEngine.getWafflesSettings().getFloat("streaming:latency", 0, 0.2f);
	mvs.mVideoStreamingLatency = streamLatency;

	mvs.mPdfCanShowLinks	   = true;
	mvs.mPdfLinkTappedCallback = [this](ds::pdf::PdfLinkInfo linkInfo) {
		ds::model::ContentModelRef fakeThing;
		fakeThing.setPropertyResource("media", ds::Resource(linkInfo.mUrl)); // TODO: cannot tell if this wants mMediaPropertyKey
		mEventClient.notify(RequestViewerLaunchEvent(
			ViewerCreationArgs(fakeThing, VIEW_TYPE_TITLED_MEDIA_VIEWER, getCenterPosition())));
	};

	mMediaPlayer->setSettings(mvs);

	if (!mShowingWebcam) {
		mRootLayout->setContentModel(mMediaRef);


		if (primaryResource.getType() == ds::Resource::VIDEO_TYPE && primaryResource.getThumbnailId() > 0) {
			auto thumbResource = mEngine.mContent.getChildByName("sqlite.resources")
									 .getChildById(primaryResource.getThumbnailId())
									 .getPropertyResource("resourcesid");

			auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("thumb_hodler");
			auto vidThumb  = mRootLayout->getSprite<ds::ui::Image>("video_thumb");
			if (theLayout && vidThumb && !thumbResource.empty()) {
				mShowingVideo = true;
				mMediaPlayer->setSize(primaryResource.getWidth(), primaryResource.getHeight());
				mMediaPlayer->setContentAspectRatio(primaryResource.getWidth() / primaryResource.getHeight());
				theLayout->show();
				vidThumb->setImageResource(thumbResource);
				vidThumb->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos) { startVideo(); });
				vidThumb->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
					if (ti.mPhase == ds::ui::TouchInfo::Moved) {
						if (ti.mFingerIndex > 0 ||
							glm::distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()) {
							bs->passTouchToSprite(this, ti);
							return;
						}
					}
				});
				vidThumb->enable(true);
				vidThumb->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
			}
		} else {
			mShowingVideo = false;

			auto prePipe  = mEngine.getAppSettings().getString("streaming:pipline:pre", 0, "");
			prePipe = mEngine.getWafflesSettings().getString("streaming:pipline:pre", 0, prePipe);
			auto postPipe = mEngine.getAppSettings().getString("streaming:pipline:post", 0, "");
			postPipe = mEngine.getWafflesSettings().getString("streaming:pipline:post", 0, postPipe);

			// Testing some additional streaming options
			if (!prePipe.empty() && !postPipe.empty()) {
				if (primaryResource.getType() == ds::Resource::VIDEO_STREAM_TYPE) {
					auto modifiedResource = primaryResource;
					modifiedResource.setLocalFilePath(prePipe + primaryResource.getAbsoluteFilePath() + " " + postPipe);
					mMediaPlayer->loadMedia(modifiedResource);
				} else {
					mMediaPlayer->loadMedia(primaryResource);
				}
			} else {
				mMediaPlayer->loadMedia(primaryResource);
			}
		}
	}


	auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mMediaPlayer->getPlayer());
	if (webPlayer && webPlayer->getWeb()) {

		ds::ui::WebInterface* webInterface = dynamic_cast<ds::ui::WebInterface*>(webPlayer->getWebInterface());
		ds::ui::ImageButton*  keyboardBtn  = webInterface->getKeyboardButton();

		webPlayer->setKeyboardStateCallback([this, webPlayer, keyboardBtn](const bool onScreen) {
			if (onScreen) {
				ci::Color lightGrey = mEngine.getColors().getColorFromName("ui_icon_background");
				auto	  keeb		= webPlayer->getWebInterface()->getSoftKeyboard();
				auto&	  setty		= keeb->getSoftKeyboardSettings();

				setty.mKeyDownColor				  = ci::Color::black();
				setty.mKeyUpColor				  = ci::Color(lightGrey);
				setty.mGraphicType				  = ds::ui::SoftKeyboardSettings::kSolid;
				setty.mGraphicRoundedCornerRadius = 0;
				keeb->setSoftKeyboardSettings(setty);

				setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard on_64.png", keyboardBtn);

				mShowingKeyboard = true;
				hideTitle();

				float keyboardHeight = mEngine.getAppSettings().getFloat("media_viewer:keyboard_height", 0, 420.0f);
				keyboardHeight = mEngine.getWafflesSettings().getFloat("media_viewer:keyboard_height", 0, keyboardHeight);
				mBoundingArea.y2	 = mEngine.getWorldHeight() - keyboardHeight;
				if (getPosition().y + getHeight() + keyboardHeight > mEngine.getWorldHeight()) {
					tweenStarted();
					tweenPosition(
						ci::vec3(getPosition().x, mEngine.getWorldHeight() - getHeight() - keyboardHeight, 0.0f),
						mEngine.getAnimDur(), 0.0f, ci::easeInOutQuad, [this] { tweenEnded(); });
				} else {
					checkBounds(false);
				}
			} else if (!onScreen) {
				mShowingKeyboard = false;

				setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard_64.png", keyboardBtn);
				// showTitle();
			}
		});

		if (auto webby = webPlayer->getWeb()) {

			if (gifSpecial) {
				webby->setZoom(1.f);
			} else {
				auto zoom = mEngine.getAppSettings().getFloat("web:default_waffles", 0, 1.0f);
				zoom	  = mEngine.getWafflesSettings().getFloat("web:default_waffles", 0, zoom);
				webby->setZoom(zoom);
			}

			webby->setTitleChangedFn([this](const std::wstring& title) {
				mRootLayout->setSpriteText("name", title);
				layout();
				// mRootLayout->runLayout();
			});

			webby->setFullscreenChangedCallback([this](bool isFullscreen) {
				if (isFullscreen && !getIsFullscreen()) {
					mEventClient.notify(RequestFullscreenViewer(this));
				} else if (!isFullscreen && getIsFullscreen()) {
					mEventClient.notify(RequestUnFullscreenViewer(this));
				}
			});
		}
	}

	if (auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(mMediaPlayer->getPlayer())) {
		if (mMediaRef.getPropertyInt("page_number") != 0) {
			if (mMediaRef.getPropertyInt("page_number") > pdfPlayer->getPageCount()) {
				pdfPlayer->callAfterDelay([pdfPlayer, this] { pdfPlayer->setPageNum(pdfPlayer->getPageCount()); },
										  0.1f);
			} else {
				pdfPlayer->callAfterDelay(
					[pdfPlayer, this] { pdfPlayer->setPageNum(mMediaRef.getPropertyInt("page_number")); }, 0.1f);
			}
		}

		pdfPlayer->setDoubleTapCallback([this](ds::ui::Sprite*, const ci::vec3&) {
			if (getIsFullscreen()) {
				mEventClient.notify(RequestUnFullscreenViewer(this));
			} else {
				mEventClient.notify(RequestFullscreenViewer(this));
			}
		});
	}

	if (!mShowingWebcam) {
		ContentUtils::setMediaInterfaceStyle(mMediaPlayer->getMediaInterface());
	}

	// setting size is necessary to get size limits to work
	calculateSizeLimits();

	if (!mShowingWebcam) {
		if (getWidth() < 1.0f || getHeight() < 1.0f ||
			(!mShowingVideo && (!mMediaPlayer->getInitialized() || mInitialLoadError))) {
			// mRootLayout->setSpriteText("name", "Sorry! Couldn't load this media type or file: " +
			// primaryResource.getAbsoluteFilePath() + " Title: " + mMediaRef.getPropertyString("name"));

			mContentAspectRatio = 1.0f;

			setSize(300.0f, 300.0f);
			setSizeLimits();
			setViewerSize(300.0f, 300.0f);
			//showTitle();
			//mCanResize	= false;
			mFatalError = true;

			ds::model::ContentModelRef errorModel;

			std::string errorMessage = "We couldn't load this piece of media because ";
			if (ds::safeFileExistsCheck(primaryResource.getAbsoluteFilePath())) {
				errorMessage.append("the size of the media was not found.");
			} else {
				errorMessage.append("the media file couldn't be found on the hard disk.");
			}

			errorModel.setProperty("name", std::string("Sorry!"));
			errorModel.setProperty("error", errorMessage);
			errorModel.setPropertyResource("media", primaryResource); // TODO: cannot tell if this wants mMediaPropertyKey
			errorModel.setProperty("media_path", primaryResource.getAbsoluteFilePath());
			errorModel.setProperty("media_name", mMediaRef.getPropertyString("name"));

			callAfterDelay(
				[this, errorModel] {
					mEventClient.notify(RequestViewerLaunchEvent(ViewerCreationArgs(
						errorModel, VIEW_TYPE_ERROR, getCenterPosition(), ViewerCreationArgs::kViewLayerTop)));
					if (mCloseRequestCallback) mCloseRequestCallback();
				},
				0.01f);

		} else {
			mFatalError = false;
		}

		mMediaPlayer->enter();
	}

	layout();
	// mRootLayout->runLayout();


	if (!mFatalError) {
		if (mMediaRef.getPropertyResource(mMediaPropertyKey).getType() == ds::Resource::VIDEO_TYPE ||
			mMediaRef.getPropertyResource(mMediaPropertyKey).getType() == ds::Resource::YOUTUBE_TYPE) {
			mShowingVideo = true;

			if (mCreationArgs.mCloseOnVideoComplete) {
				auto videoPlayer = dynamic_cast<ds::ui::VideoPlayer*>(mMediaPlayer->getPlayer());
				if (videoPlayer) {
					videoPlayer->setVideoCompleteCallback([this] {
						if (mCloseRequestCallback) mCloseRequestCallback();
					});
				}
			}


		} else {
			mShowingVideo = false;
		}

		if (mMediaRef.getPropertyResource(mMediaPropertyKey).getType() == ds::Resource::WEB_TYPE) {
			mShowingWeb = true;
		} else {
			mShowingWeb = false;
		}
	}

	if (mShowingWeb) {
		if (auto outy = mRootLayout->getSprite("player_outer")) {
			if (auto player = mRootLayout->getSprite("media_player")) {
				outy->addChildPtr(player);
			}

			if (auto capt = mRootLayout->getSprite("capture_player")) {
				outy->addChildPtr(capt);
			}
		}

		if (auto filly = mRootLayout->getSprite("bg_filler")) {
			filly->setOpacity(1.f);
		}
	}
}

void TitledMediaViewer::startVideo() {
	if (!mMediaPlayer || !mRootLayout) return;
	auto mvs			 = mMediaPlayer->getSettings();
	auto mMediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
	auto primaryResource = mMediaRef.getPropertyResource(mMediaPropertyKey);

	if (primaryResource.getType() != ds::Resource::VIDEO_TYPE &&
		primaryResource.getType() != ds::Resource::YOUTUBE_TYPE) {
		return;
	}

	mvs.mVideoAutoPlayFirstFrame = false;
	mMediaPlayer->setSettings(mvs);

	auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("thumb_hodler");
	auto vidThumb  = mRootLayout->getSprite<ds::ui::Image>("video_thumb");
	if (theLayout && vidThumb) {
		theLayout->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [theLayout] { theLayout->hide(); });
		vidThumb->enable(false);
	}

	mMediaPlayer->loadMedia(primaryResource);
	ContentUtils::setMediaInterfaceStyle(mMediaPlayer->getMediaInterface());
	mRootLayout->runLayout();
	mMediaPlayer->enter();
}

void TitledMediaViewer::onLayout() {

	if (mMediaPlayer) {
		float w = getWidth();
		float h = getHeight();


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
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
		mRootLayout->clearAnimateOnTargets(true);
	}
}

void TitledMediaViewer::layoutHotspots() {
	for (auto it : mHotspots) {
		auto	 thisCm = it->getContentModel();
		ci::vec2 pos	= ci::vec2(thisCm.getPropertyFloat("hotspot_x"), thisCm.getPropertyFloat("hotspot_y"));
		ci::vec2 size	= ci::vec2(thisCm.getPropertyFloat("hotspot_w"), thisCm.getPropertyFloat("hotspot_h"));
		bool	 noXY	= (pos.x == 0.f && pos.y == 0.f);
		bool	 noWH	= (size.x == 0.f && size.y == 0.f);

		std::string theStyle = "rectangles";
		if (noXY && noWH) {
			size = ci::vec2(1.f);
		} else if (noWH) {
			theStyle = "spots";
		}
		it->setPosition(pos * ci::vec2(mMediaPlayer->getSize()));
		it->setSize(size * ci::vec2(mMediaPlayer->getSize()));
		it->runLayout();
	}
}

void TitledMediaViewer::loadHotspots() {
	for (auto it : mHotspots) {
		it->release();
	}

	mHotspots.clear();

	for (auto hs : mMediaRef.getChildren()) {
		ci::vec2 pos  = ci::vec2(hs.getPropertyFloat("hotspot_x"), hs.getPropertyFloat("hotspot_y"));
		ci::vec2 size = ci::vec2(hs.getPropertyFloat("hotspot_w"), hs.getPropertyFloat("hotspot_h"));
		bool	 noXY = (pos.x == 0.f && pos.y == 0.f);
		bool	 noWH = (size.x == 0.f && size.y == 0.f);

		std::string theStyle = "rectangles";
		if (noXY && noWH) {
			size = ci::vec2(1.f);
		} else if (noWH) {
			theStyle = "spots";
		}

		auto layoutFile = "waffles/viewer/hotspot_rectangle.xml";


		auto hotspot = new ds::ui::SmartLayout(mEngine, layoutFile);
		hotspot->setContentModel(hs);
		addChildPtr(hotspot);
		mHotspots.emplace_back(hotspot);

		hotspot->setTapCallback([this, hotspot](ds::ui::Sprite* bs, const ci::vec3& pos) {
			auto helper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper>();
			auto field_name = mEngine.getWafflesSettings().getString("hotspot:destination:field_name", 0, "destination");
			auto destId = hotspot->getContentModel().getPropertyString(field_name);
			if (!destId.empty()) {
				// launch the thing for the hotspot at pos
				ds::model::ContentModelRef linkMedia = helper->getRecordByUid(destId);
				if (linkMedia.empty()) {
					DS_LOG_WARNING("Hotspot node not found for id == " << destId);
				} else {
					
					if (ContentUtils::getDefault(mEngine)->isMedia(linkMedia)) {
						mEventClient.notify(RequestViewerLaunchEvent(
							ViewerCreationArgs(linkMedia, VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
					} else {
						mEventClient.notify(waffles::RequestEngagePresentation(linkMedia));
					}
				}
			} else if (!hotspot->getContentModel().getChildren().empty()) {
				auto position = pos;
				for (auto child : hotspot->getContentModel().getChildren()) {
					mEventClient.notify(
						RequestViewerLaunchEvent(ViewerCreationArgs(child, VIEW_TYPE_TITLED_MEDIA_VIEWER, position)));
					position = position + mEngine.getWafflesSettings().getVec3("launcher:presentation:hotspot:asset:offset",
																		   0, ci::vec3(45, 45, 0));
				}
			} else {
				// Not a valid hotspot
				DS_LOG_INFO("Invalid hotspot encountered! UID: " << destId);
			}
			// if (destId < 1) return; // nothing specified, skip it
		});

		if (mCreationArgs.mTouchEvents) {
			hotspot->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
				if (ti.mPhase == ds::ui::TouchInfo::Moved &&
					glm::distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()) {
					bs->passTouchToSprite(this, ti);
					return;
				}
			});
		}
	}

	layoutHotspots();
}

void TitledMediaViewer::onCreationArgsSet() {
	if (mMediaPlayer) {
		auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(mMediaPlayer->getPlayer());
		if (pdfPlayer && pdfPlayer->getPDF()) {
			pdfPlayer->getPDF()->setPageNum(mCreationArgs.mPage);
			pdfPlayer->getPDFInterface()->setColor(ci::Color(1, 0, 0));
		}


		if (mCreationArgs.mAutoStart && mShowingVideo) {
			startVideo();
		}

		auto vidPlayer = dynamic_cast<ds::ui::VideoPlayer*>(mMediaPlayer->getPlayer());
		if (vidPlayer) {
			vidPlayer->setVideoLoop(mCreationArgs.mLooped);
			auto video = vidPlayer->getVideo();
			if (video) {
				if (mCreationArgs.mVideoTimePosition > 0.0) {
					video->seekPosition(mCreationArgs.mVideoTimePosition);
				}
				video->setVolume((float)(mCreationArgs.mVolume) / 100.0f);
				if (!mCreationArgs.mAutoStart) {
					// vidPlayer->setResetOnVideoComplete(true);
					if (mCreationArgs.mVideoTimePosition > 0.0) {
						video->playAFrame(-1.0, nullptr, false);
					} else {
						video->playAFrame();
					}
				}
				video->setMute(mCreationArgs.mMuted);
			}
		}
	}

	if (!mCreationArgs.mTouchEvents) {
		enable(false);
		if (mMediaPlayer) {
			mMediaPlayer->enable(!mCreationArgs.mTouchEvents && mCreationArgs.mStartLocked);
		}

		if (auto background = mRootLayout->getSprite("background")) {
			background->enable(false);
		}
	} else {
	}

	if (mCreationArgs.mUseHotspots) {
		loadHotspots();
	}


	if (mCreationArgs.mStartDrawing) {
		callAfterDelay(
			[this] {
				showTitle();
				toggleDrawing();
			},
			mEngine.getAnimDur());
	}
}


void TitledMediaViewer::onFullscreenSet() {
	if (mIsFullscreen) {
		hideTitle();
		hideInnerSideBar();
		if (mMediaPlayer) {

			ds::ui::MediaInterface* mediaInterface = mMediaPlayer->getMediaInterface();

			if (mediaInterface) {
				mediaInterface->setAllowDisplay(false);
				mediaInterface->animateOff();
			}
			mMediaPlayer->hideInterface();
		}

		/* tweenNormalized(
			getAnimateDuration(), 0.f, ci::easeNone, [this] { layout(); }, [this] { layout(); }); */
	} else {
		showTitle();
		showInnerSideBar();
		if (mMediaPlayer) {

			ds::ui::MediaInterface* mediaInterface = mMediaPlayer->getMediaInterface();

			if (mediaInterface) {
				mediaInterface->setAllowDisplay(true);
				mediaInterface->animateOn();

				// coming back from fullscreen, we gotta re-link the website
				// to get the auth callback back
				if (mShowingWeb) {
					auto webPlayer	  = dynamic_cast<ds::ui::WebPlayer*>(mMediaPlayer->getPlayer());
					auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaPlayer->getMediaInterface());
					if (webPlayer && webPlayer->getWeb() && webInterface) {
						auto webby = webPlayer->getWeb();
						if (webby) {
							webInterface->linkWeb(webby);
						}
					}
				}
			}

			mMediaPlayer->showInterface();
		}
	}

	// Either way, send keyboard input to this one!
	auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mMediaPlayer->getPlayer());
	if (webPlayer && webPlayer->getWeb()) {
		mEngine.registerEntryField(webPlayer->getWeb());
	}
}

void TitledMediaViewer::toggleDrawing() {
	// if (!mShowingTitle && !mIsFullscreen) return;

	mDrawingMode = !mDrawingMode;

	if (mDrawingMode) {
		hideTitle();
		if (!mDrawingArea) {
			float widdy = mEngine.getAppSettings().getFloat("drawing:initial_resolution", 0, 3000.0f);
			widdy = mEngine.getWafflesSettings().getFloat("drawing:initial_resolution", 0, widdy);

			float asp	= getWidth() / getHeight();

			mDrawingArea = new DrawingArea(mEngine, widdy, widdy / asp);
			mDrawingArea->setOpacity(0.0f);
			addChildPtr(mDrawingArea);
		}

		mDrawingArea->show();
		mDrawingArea->tweenOpacity(1.0f, getAnimateDuration(), 0.0f);

		if (auto drawingBtn = mRootLayout->getSprite<ds::ui::LayoutButton>("drawing.the_button")) {
			drawingBtn->showDown();
		}

		if (mMediaPlayer) {
			// let streams keep on playing
			auto mMediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
			if (mMediaRef.getPropertyResource(mMediaPropertyKey).getType() != ds::Resource::VIDEO_STREAM_TYPE) {
				mMediaPlayer->pauseContent();
			}
			mMediaPlayer->hideInterface();
		}

		layout();

		if (!mIsFullscreen) {
			onLayout();
			// setSize(getWidth(), getHeight()+mDrawingArea->getControlHeight());
			//  mBoundingArea.inflate(ci::vec2(0.f, -mDrawingArea->getControlHeight()));
			//  mBoundingArea.y2 -= (mDrawingArea->getControlHeight());
			//  checkBounds(false);
		}

	} else {
		if (!mIsFullscreen) showTitle();
		if (auto drawingBtn = mRootLayout->getSprite<ds::ui::LayoutButton>("drawing.the_button")) {
			drawingBtn->showUp();
		}

		if (mDrawingArea) {
			mDrawingArea->tweenOpacity(0.0f, getAnimateDuration(), 0.0f, ci::easeNone,
									   [this] { mDrawingArea->hide(); });
		}

		if (mMediaPlayer && !mIsFullscreen) {
			mMediaPlayer->showInterface();
		}

		if (!mIsFullscreen) {
			// setSize(getWidth(), getHeight()+mDrawingArea->getControlHeight());
			onLayout();
			// mBoundingArea.inflate(ci::vec2(0.f, mDrawingArea->getControlHeight()));
			// mBoundingArea.y2 += /*getHeight() -*/ mDrawingArea->getControlHeight();
			// checkBounds(false);
		}
	}
}

void TitledMediaViewer::playContent() {
	if (mMediaPlayer) {
		mMediaPlayer->playContent();
	}
}

void TitledMediaViewer::pauseContent() {
	if (mMediaPlayer) {
		mMediaPlayer->pauseContent();
	}
}

void TitledMediaViewer::toggleMute() {
	if (mMediaPlayer) {
		mMediaPlayer->toggleMute();
	}
}

void TitledMediaViewer::mute()
{
	if (mMediaPlayer) {
		mMediaPlayer->mute();
	}
}


void TitledMediaViewer::unmute() {
	if (mMediaPlayer) {
		mMediaPlayer->unmute();
	}
}

waffles::ViewerCreationArgs TitledMediaViewer::getDuplicateCreationArgs() {

	ViewerCreationArgs args = ViewerCreationArgs(mMediaRef, VIEW_TYPE_TITLED_MEDIA_VIEWER,
												 ci::vec3(getPosition().x, getPosition().y, getPosition().z),
												 getViewerLayer(), getScaleWidth(), false, getIsFullscreen(), true);
	args.mLooped			= mCreationArgs.mLooped;
	args.mAutoStart			= mCreationArgs.mAutoStart;

	auto thePlayer = getMediaPlayer()->getPlayer();
	auto vidPlayer = dynamic_cast<ds::ui::VideoPlayer*>(thePlayer);
	if (vidPlayer && vidPlayer->getVideo()) {
		auto theVideo			= vidPlayer->getVideo();
		args.mVideoTimePosition = theVideo->getCurrentPosition();
		args.mVolume			= (int)roundf(theVideo->getVolume() * 100.0f);
		args.mAutoStart			= theVideo->getIsPlaying();
		args.mMuted				= theVideo->getIsMuted();
		args.mLooped			= theVideo->getIsLooping();
	}

	auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(thePlayer);
	if (pdfPlayer && pdfPlayer->getPDF()) {
		args.mPage = pdfPlayer->getPDF()->getPageNum();
	}

	return args;
}

void TitledMediaViewer::setInterfaceLocked(bool isLocked) {
	if (!mMediaPlayer) return;

	if (auto iFace = dynamic_cast<ds::ui::WebInterface*>(mMediaPlayer->getMediaInterface())) {
		if (isLocked) {
			iFace->startTouch();
		} else {
			iFace->stopTouch();
		}
	} else if (auto iFace = dynamic_cast<ds::ui::PDFInterface*>(mMediaPlayer->getMediaInterface())) {
		if (isLocked) {
			iFace->startTouch();
		} else {
			iFace->stopTouch();
		}
	}
}

void TitledMediaViewer::rotateMedia() {
	if (!mMediaPlayer || mAnimating) return;

	mMediaRotation++;
	if (mMediaRotation > 3) mMediaRotation = 0;

	while (mMediaPlayer->getRotation().z > 180.0f) {
		mMediaPlayer->setRotation(mMediaPlayer->getRotation().z - 360.0f);
	}

	auto prevViewerPos = getPosition();
	prevViewerPos.y += getHeight();

	ci::vec3 prevPos = mMediaPlayer->getPosition();
	// mMediaPlayer->tweenRotation(ci::vec3(0.0f, 0.0f, 90.0f * (float)mMediaRotation), mEngine.getAnimDur(), 0.0f,
	// ci::easeInOutQuint);
	mMediaPlayer->setRotation(ci::vec3(0.0f, 0.0f, 90.0f * (float)mMediaRotation));

	float w = mMediaPlayer->getHeight();
	float h = mMediaPlayer->getWidth();

	if (mMediaRotation % 2 == 0) {
		w = mMediaPlayer->getWidth();
		h = mMediaPlayer->getHeight();
	}

	mContentAspectRatio = w / h;

	BasePanel::setAbsoluteSizeLimits(ci::vec2(mMinSize.x, mMinSize.x / mContentAspectRatio),
									 ci::vec2(mEngine.getWorldWidth(), mEngine.getWorldHeight()));
	setSize(w, h);
	setSizeLimits();
	setViewerSize(w, h);
	layout(); // force a layout to fix the edge case of a square asset

	ci::vec3 postPos = mMediaPlayer->getPosition();

	// mMediaPlayer->setPosition(prevPos);
	// tweenStarted();
	// mMediaPlayer->tweenPosition(postPos, mEngine.getAnimDur(), 0.0f, ci::easeInOutQuint, [this] { tweenEnded(); });

	// if(mOptionsLayout && mOptionsButton && mSidebar) {
	//	mOptionsLayout->setPosition(mOptionsButton->getPosition().x - mOptionsButton->getWidth() -
	// mOptionsLayout->getWidth(), getHeight() - mSidebar->getHeight());
	// }

	setPosition(prevViewerPos.x, prevViewerPos.y - h);

	if (mDrawingArea) {
		auto da = mDrawingArea;
		mDrawingArea->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [da] { da->release(); });
		mDrawingArea = nullptr;
	}

	if (mDrawingMode) {
		toggleDrawing();
	}
}
void TitledMediaViewer::toggleOptions() {
	if (!mShowingTitle || !mRootLayout) return;


	mShowingOptions = !mShowingOptions;

	auto optionsLayout = mRootLayout->getSprite("the_options");
	auto optionsButton = mRootLayout->getSprite<ds::ui::LayoutButton>("options.the_button");

	if (mShowingOptions) {
		if (optionsLayout) {
			optionsLayout->show();
			optionsLayout->tweenAnimateOn(true, 0.0f, 0.05f);
		}

		if (optionsButton) {
			optionsButton->showDown();
		}
	} else {
		if (optionsLayout) {
			optionsLayout->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
										[optionsLayout] { optionsLayout->hide(); });
		}

		if (optionsButton) {
			optionsButton->showUp();
		}
	}
}

void TitledMediaViewer::showTitle() {
	if (!mRootLayout || mShowingTitle || mShowingKeyboard) return;
	mShowingTitle = true;
	if (auto titleHodler = mRootLayout->getSprite("title_layout")) {
		titleHodler->show();
		titleHodler->tweenOpacity(1.0f, mEngine.getAnimDur());
	}
	onLayout();

	mControlsTimeoutTimer.timedCallback(
		[this] {
			if (mDrawingMode || mShowingKeyboard) return;
			hideTitle();
			hideInnerSideBar();
		},
		mEngine.getWafflesSettings().getFloat("media_viewer:control_timeout", 0, 5.f));
}

void TitledMediaViewer::hideTitle() {
	if (!mRootLayout || !mShowingTitle) return;
	mShowingTitle = false;
	if (auto titleHodler = mRootLayout->getSprite("title_layout")) {
		titleHodler->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
								  [titleHodler] { titleHodler->hide(); });
	}
	onLayout();

	// mBoundingArea.y2 = mEngine.getWorldHeight();
	//  checkBounds(false);
}

void TitledMediaViewer::toggleTitle() {
	if (mShowingTitle || mShowingKeyboard) {
		hideTitle();
	} else {
		showTitle();
	}
}

void TitledMediaViewer::showInnerSideBar() {
	auto innerSideBar = mRootLayout->getSprite("inner_sidebar");
	if (!mRootLayout || mShowingInnerSideBar) return;
	mShowingInnerSideBar = true;

	if (innerSideBar) {
		innerSideBar->show();
		innerSideBar->tweenOpacity(1.0f, mEngine.getAnimDur());
	}
}

void TitledMediaViewer::hideInnerSideBar() {
	auto innerSideBar = mRootLayout->getSprite("inner_sidebar");
	if (!mRootLayout || !mShowingInnerSideBar) return;
	mShowingInnerSideBar = false;

	if (innerSideBar) {
		innerSideBar->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
								   [innerSideBar] { innerSideBar->hide(); });
	}
}

void TitledMediaViewer::toggleInnerSideBar() {
	auto innerSideBar = mRootLayout->getSprite("inner_sidebar");
	if (mShowingInnerSideBar || mShowingKeyboard) {
		hideInnerSideBar();
		/*if (!mDrawingMode) {
			innerSideBar->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
									   [innerSideBar] { innerSideBar->hide(); });
		}*/
	} else {
		showInnerSideBar();
		/*innerSideBar->show();
		innerSideBar->tweenOpacity(1.0f, mEngine.getAnimDur());*/
	}
}

void TitledMediaViewer::userInputReceived() {
	BasePanel::userInputReceived();
	// Input = focus = keyboard
	auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mMediaPlayer->getPlayer());
	if (webPlayer && webPlayer->getWeb()) {
		mEngine.registerEntryField(webPlayer->getWeb());
	}

	if (mMediaPlayer && !mIsFullscreen && !mDrawingMode) {
		mMediaPlayer->showInterface();
	}

	layout();
}

void TitledMediaViewer::setKeyboardButtonImage(std::string imagePath, ds::ui::ImageButton* keyboardBtn) {
	keyboardBtn->setHighImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setNormalImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setColor(ci::Color::black());
	keyboardBtn->setCornerRadius(0.f);
}

} // namespace mv
