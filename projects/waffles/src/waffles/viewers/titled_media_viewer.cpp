#include "stdafx.h"

#include "titled_media_viewer.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/toggle_container.h>
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

#include "waffles/common/ui_utils.h"
#include "waffles/pinboard/pinboard_button.h"
#include "waffles/util/base_waffles_helper.h"
#include "waffles/util/capture_player.h"
#include "waffles/util/shadow_layout.h"
#include "waffles/util/waffles_helper.h"
#include "waffles/waffles_events.h"

#include <ds/content/content_helper.h>
#include <ds/ui/media/media_interface_builder.h>
#include <ds/app/engine/engine_events.h>

namespace waffles {

TitledMediaViewer::TitledMediaViewer(ds::ui::SpriteEngine& g, const std::string& eventChannel,
									 const std::string& layoutPath)
  : BaseElement(g)
  , mPlayerLoadedTimer(mEngine)
  , mControlsTimeoutTimer(mEngine) {

	BaseElement::setChannelName(eventChannel);
	if (eventChannel.empty()) {
		mEventClient.setNotifier(g.getNotifier());
	} else {
		mEventClient.setNotifier(g.getChannel(eventChannel));
	}
	mEventClient.start();

	mViewerType	  = VIEW_TYPE_TITLED_MEDIA_VIEWER;
	mAnimDuration = mEngine.getAnimDur();

	const float minSize = mEngine.getWafflesSettings().getFloat("media_viewer:min_size", 0, 400.0f);
	mAbsMinSize.x		= minSize;
	mAbsMinSize.y		= minSize;

	const float maxSize = mEngine.getWafflesSettings().getFloat("media_viewer:max_size", 0, 400.0f);
	mAbsMaxSize.x		= maxSize;
	mAbsMaxSize.y		= maxSize;

	mFit = ds::ui::Fit(ds::ui::Fit::Align::X_MID_Y_MID, ds::ui::Fit::MeetOrSlice::MEET);

	/* setTransparent(false);
	setColor(ci::Color(1.f, 0.f, 1.f)); */

	auto tapCallback = [this](Sprite* bs, const ci::vec3& pos) {
		if (mIsFullscreen) {
			hideTitle();
			hideInnerSideBar();
			mEventClient.notify(RequestViewerLaunchEvent(ViewerCreationArgs(
				ContentModelRef(), VIEW_TYPE_FULLSCREEN_CONTROLLER, pos, ViewerCreationArgs::kViewLayerTop)));
		} else {
			showTitle();
			showInnerSideBar();
		}
	};
	auto doubleTapCallback = [this](Sprite* bs, const ci::vec3& pos) {
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

	mRootLayout = new ds::ui::SmartLayout(mEngine, layoutPath);
	addChildPtr(mRootLayout);
	mRootLayout->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti) {
		if (ti.mPhase == ds::ui::TouchInfo::Moved) {
			bs->passTouchToSprite(this, ti);
			return;
		}
	});
	mRootLayout->setTapCallback(tapCallback);
	mRootLayout->setDoubleTapCallback(doubleTapCallback);

	processAllowedButtons();

	auto background = mRootLayout->getSprite("background");
	if (background) {
		background->setTapCallback(tapCallback);
		background->setDoubleTapCallback(doubleTapCallback);
		background->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti) {
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
			const bool hadError = mInitialLoadError;
			mInitialLoadError	= false;
			if (!mRootLayout) return;
			if (auto placeholder = mRootLayout->getSprite("loading_placeholder")) {
				placeholder->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
										  [placeholder] { placeholder->hide(); });
			}
			if (hadError) {
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
		
		// TODO: this solves an issue of having a duplicate media controls UI
		// TODO: this finds the unnamed sprite and forcibly hides components
		// TODO: this should be further investigated to solve the creations
		// TODO: rather than the bandaid afterwards that this is
		mEngine.timedCallback(
			[this]() {
				forEachChild(
					[this](ds::ui::Sprite& s) {
						if (s.getSpriteName() == L"ui_holder") {
							s.forEachChild(
								[this](ds::ui::Sprite& ss) {
									if (ss.getSpriteName() != L"inner_sidebar") {
										ss.forEachChild(
											[this](ds::ui::Sprite& sss) {
												sss.hide();
											},
											true
										);
									}
								},
								false
							);
						}
					},
					true
				);
			},
			0.01f
		);
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
		auto mediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(particleBackground);
		particleBackground.setPropertyResource(
			mediaPropertyKey,
			getMedia().getPropertyResource(mediaPropertyKey)); // TODO: cannot tell if this wants mMediaPropertyKey
		mEventClient.notify(RequestBackgroundChange(BACKGROUND_TYPE_PARTICLES, ContentModelRef()));
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

	mRootLayout->setSpriteClickFn("detach.the_button", [this] {
		if (getIsDetached()) {
			mEventClient.notify(RequestAttachViewer(this));
		} else {
			mEventClient.notify(RequestDetachViewer(this));
		}
	});

	/// This is an escape hatch for video players when drawing is happening and the mode changes
	/// e.g. the idle timeout occurs, or an ambient button is pressed etc.
	/// The sprite will likely still exist and be an orphan, which is likely a big TODO.
	mEventClient.listenToEvents<RequestCloseAllEvent>([this](auto& e) {
		if (mMediaPlayer) {
			mMediaPlayer->pauseContent();
			mMediaPlayer->mute();
		}
		cleanupDrawing(true);
	});
}


void TitledMediaViewer::calculateSizeLimits() {
	if (!mMediaPlayer) return;

	auto width	= getWidth();
	auto height = mContentAspectRatio = mMediaPlayer->getContentAspectRatio();

	float contentWidth	= mMediaPlayer->getWidth();
	float contentHeight = mMediaPlayer->getHeight();

	if (contentWidth < 1.0f) contentWidth = 1.0f; // prevents divide-by-zero errors
	if (contentHeight < 1.0f) contentHeight = 1.0f;

	// calculate a default size that maximizes size
	float settingsAspect = 1.0f;
	float settingsWidth	 = mMediaPlayer->getSettings().mDefaultBounds.x;
	float settingsHeight = mMediaPlayer->getSettings().mDefaultBounds.y;
	/* if(mShowingWebCam){
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
	if (mDefaultSize.x == 0 || mDefaultSize.y == 0) {
		DS_LOG_ERROR("DEFAULT SIZE IS ZERO");
	}
}

void TitledMediaViewer::onMediaSet() {
	auto helper = ContentHelperFactory::getDefault<WafflesHelper>();

	auto newMediaRef = helper->getRecordByUid(mMediaRef.getUid());
	if (newMediaRef) {
		mMediaRef = newMediaRef;
	}


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


	// check for a stream and do special handling.
	auto isStream		= helper->isValidStream(mMediaRef, WafflesHelper::WAFFLESCATEGORY);
	auto isStreamSource = helper->isValidStreamSource(mMediaRef, WafflesHelper::WAFFLESCATEGORY);
	if (isStream || isStreamSource) {
		ds::Resource fakeRes;
		fakeRes.setWidth(1920);
		fakeRes.setHeight(1080);

		// get the stream source
		auto streamSource = mMediaRef;
		if (isStream) {
			streamSource = helper->getStreamSourceForStream(mMediaRef, WafflesHelper::WAFFLESCATEGORY);
		}

		if (!streamSource.empty()) { // TODO figure out what to do in case our stream source is empty
			auto streamAddressKey = helper->getStreamSourceAddressKey(streamSource, WafflesHelper::WAFFLESCATEGORY);
			auto streamTypeKey	  = helper->getStreamSourceTypeKey(streamSource, WafflesHelper::WAFFLESCATEGORY);
			auto streamType		  = streamSource.getPropertyString(streamTypeKey);
			auto streamAddress	  = streamSource.getPropertyString(streamAddressKey);

			// the stream type needs to be set before the filenames
			if (streamType == "rtsp") {
				fakeRes.setType(ds::Resource::VIDEO_STREAM_TYPE);
			}
			fakeRes.setFileName(streamAddress);
			fakeRes.setLocalFilePath(streamAddress);

			if (streamType == "rtsp") {
				DS_LOG_INFO("Got a network stream! " << streamAddress);

			} else if (streamType == "capture") {
				DS_LOG_INFO("Got a capture stream! " << streamAddress);

				// set up gstreamer to capture.
				//--check for a size at the end of the stream address in the form of address@widthxheight
				std::string address = streamAddress;
				std::string size;
				float		width  = 0;
				float		height = 0;
				auto		atPos  = address.find("@");
				if (atPos != std::string::npos) {
					size	= address.substr(atPos + 1);
					address = address.substr(0, atPos);

					// split size string into width and height floats
					auto sizeParts = ds::split(size, "x");
					if (sizeParts.size() == 2) {
						width  = ds::string_to_float(sizeParts.at(0));
						height = ds::string_to_float(sizeParts.at(1));
					}
				}

				if (width == 0 || height == 0) {
					auto sz = ds::ui::GstVideo::getResolutionForCapture(address, mEngine);
					width	= sz.x;
					height	= sz.y;
				}

				std::stringstream ss;
				ss << "mfvideosrc device-name=\"" << address
				   << "\" ! queue leaky=1 max-size-buffers=0 ! videoconvert ! appsink name=appsink0";
				std::string pipeline = ss.str();
				DS_LOG_INFO("Pipeline: " << pipeline);
				fakeRes.setFileName(pipeline);
				fakeRes.setLocalFilePath(pipeline);
				fakeRes.setType(ds::Resource::VIDEO_STREAM_TYPE);
				fakeRes.setWidth(width);
				fakeRes.setHeight(height);
			}
		}
		auto mediaPropKey = helper->getMediaPropertyKey(mMediaRef);
		mMediaRef.setPropertyResource(mediaPropKey, ds::Resource(fakeRes));
	}

	auto mediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
	// DS_LOG_INFO("Titled Media Viewer | mMediaPropertyKey " << mMediaPropertyKey.c_str());
	auto primaryResource = mMediaRef.getPropertyResource(mediaPropertyKey);
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
	mvs.mPdfStartTouchable	  = mCreationArgs.mStartLocked;
	if (mShowingWebCam) {
		mvs.mDefaultBounds = ci::vec2(1920, 1080);
	}

	auto webSize = mEngine.getWafflesSettings().getVec2("web:default_size", 0, ci::vec2(-1.0f, -1.0f));
	if (primaryResource.getAbsoluteFilePath().find(".gif") != std::string::npos) {
		try {
			// Awkwardly load the image from the network so we know how big to make it
			auto imgy = loadImage(ci::loadUrl(primaryResource.getAbsoluteFilePath()));
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
	mvs.mPdfLinkTappedCallback = [this, mediaPropertyKey](ds::pdf::PdfLinkInfo linkInfo) {
		if (linkInfo.mUrl.empty()) {
			int targetPage = linkInfo.mPageDest;
			if (targetPage == 0) {
				// Try to dig the page # out of the raw URI
				for (auto pair : ds::split(linkInfo.mRawUri, "&")) {
					if (pair.find("page=") != std::string::npos) {
						auto both = ds::split(pair, "=");
						if (both.size() == 2) {
							targetPage = ds::string_to_int(both.at(1));
							break;
						}
					}
				}
			}
			// Go to PDF page?
			if (auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(mMediaPlayer->getPlayer())) {
				pdfPlayer->setPageNum(targetPage);
			}
		} else {
			ContentModelRef fakeThing;
			fakeThing.setPropertyResource(
				mediaPropertyKey,
				ds::Resource(linkInfo.mUrl)); // TODO: cannot tell if this wants mMediaPropertyKey
			auto vca		= ViewerCreationArgs(fakeThing, VIEW_TYPE_TITLED_MEDIA_VIEWER, getCenterPosition());
			vca.mCanAttach	= false;
			vca.mCanDetach	= false;
			vca.mIsDetached = true;
			mEventClient.notify(RequestViewerLaunchEvent(vca));
		}
	};

	mMediaPlayer->setSettings(mvs);

	if (!mShowingWebCam) {
		mRootLayout->setContentModel(mMediaRef);


		if (primaryResource.getType() == ds::Resource::VIDEO_TYPE && primaryResource.getThumbnailId() > 0) {
			auto thumbResource = mEngine.mContent.getChildByName("sqlite.resources")
									 .getChildById(primaryResource.getThumbnailId())
									 .getPropertyResource("resourcesid");

			auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("thumb_hodler"); // TODO fix typo
			auto vidThumb  = mRootLayout->getSprite<ds::ui::Image>("video_thumb");
			if (theLayout && vidThumb && !thumbResource.empty()) {
				mShowingVideo = true;
				mMediaPlayer->setSize(primaryResource.getWidth(), primaryResource.getHeight());
				mMediaPlayer->setContentAspectRatio(primaryResource.getWidth() / primaryResource.getHeight());
				theLayout->show();
				vidThumb->setImageResource(thumbResource);
				vidThumb->setTapCallback([this](Sprite* bs, const ci::vec3& pos) { startVideo(); });
				vidThumb->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti) {
					if (ti.mPhase == ds::ui::TouchInfo::Moved) {
						if (ti.mFingerIndex > 0 ||
							distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()) {
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

			auto prePipe = mEngine.getAppSettings().getString("streaming:pipline:pre", 0, "");			// TODO fix typo
			prePipe		 = mEngine.getWafflesSettings().getString("streaming:pipline:pre", 0, prePipe); // TODO fix typo
			auto postPipe = mEngine.getAppSettings().getString("streaming:pipline:post", 0, "");		// TODO fix typo
			postPipe = mEngine.getWafflesSettings().getString("streaming:pipline:post", 0, postPipe);	// TODO fix typo

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
		auto				  keyboardBtn  = webInterface->getKeyboardButton();

		webPlayer->setKeyboardStateCallback([this, webPlayer, keyboardBtn](const bool onScreen) {
			if (onScreen) {
				auto wafflesHelper = ContentHelperFactory::getDefault<BaseWafflesHelper>();

				ci::Color lightGrey = mEngine.getColors().getColorFromName("ui_icon_background");
				auto	  keeb		= webPlayer->getWebInterface()->getSoftKeyboard();
				if (mEngine.getAppSettings().getBool("keyboard:override_settings", 0, false)) {
					auto kbs = ds::ui::SoftKeyboardSettings();
					kbs.mGraphicKeys = false;
					keeb->setSoftKeyboardSettings(kbs);
				}
				wafflesHelper->setKeyboardStyle(keeb);

				keyboardBtn->setChecked(true);

				mShowingKeyboard = true;
				hideTitle();

				float keyboardHeight = mEngine.getAppSettings().getFloat("media_viewer:keyboard_height", 0, 420.0f);
				keyboardHeight =
					mEngine.getWafflesSettings().getFloat("media_viewer:keyboard_height", 0, keyboardHeight);
				mBoundingArea.y2 = mEngine.getWorldHeight() - keyboardHeight;
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

				keyboardBtn->setChecked(false); // Set the button to unchecked when keyboard is off screen
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

			if (mEngine.getWafflesSettings().getBool("media_viewer:web:trigger_waffles_fullscreen", 0, "true")) {
				webby->setFullscreenChangedCallback([this](bool isFullscreen) {
					if (isFullscreen && !getIsFullscreen()) {
						mEventClient.notify(RequestFullscreenViewer(this));
					} else if (!isFullscreen && getIsFullscreen()) {
						mEventClient.notify(RequestUnFullscreenViewer(this));
					}
				});
			}
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

		pdfPlayer->setDoubleTapCallback([this](Sprite*, const ci::vec3&) {
			if (getIsFullscreen()) {
				mEventClient.notify(RequestUnFullscreenViewer(this));
			} else {
				mEventClient.notify(RequestFullscreenViewer(this));
			}
		});
	}

	if (!mShowingWebCam) {
		auto mediaInterface = ds::ui::MediaInterfaceBuilder::buildMediaInterface(
			mEngine,
			mMediaPlayer->getPlayer(),
			mRootLayout->getSprite("ui_holder")
		);

		auto wafflesHelper = ContentHelperFactory::getDefault<WafflesHelper>();
		if (wafflesHelper) {
			wafflesHelper->setMediaInterfaceStyle(mediaInterface);
		}
		// ContentUtils::setMediaInterfaceStyle(mMediaPlayer->getMediaInterface());
	}

	// setting size is necessary to get size limits to work
	calculateSizeLimits();

	if (!mShowingWebCam) {
		if (getWidth() < 1.0f || getHeight() < 1.0f ||
			(!mShowingVideo && (!mMediaPlayer->getInitialized() || mInitialLoadError))) {
			mFatalError	 = true;
			mMediaPlayer = nullptr;
		} else {
			mFatalError = false;
		}

		if (mMediaPlayer) mMediaPlayer->enter();
	}
	/* auto nameSp = mRootLayout->getSprite<ds::ui::Text>("name");
	if (nameSp) {

		nameSp->setResizeLimit(mMediaPlayer->getWidth(), nameSp->getResizeLimitHeight());
		auto txt = mMediaRef.getPropertyString("record_name");
		nameSp->setText("");
		nameSp->setText(txt);
	}*/

	layout();
	// mRootLayout->runLayout();


	if (!mFatalError) {
		if (mMediaRef.getPropertyResource(mediaPropertyKey).getType() == ds::Resource::VIDEO_TYPE ||
			mMediaRef.getPropertyResource(mediaPropertyKey).getType() == ds::Resource::YOUTUBE_TYPE) {
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

		if (mMediaRef.getPropertyResource(mediaPropertyKey).getType() == ds::Resource::WEB_TYPE) {
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

void TitledMediaViewer::startVideo() const {
	if (!mMediaPlayer || !mRootLayout) return;
	auto mvs			  = mMediaPlayer->getSettings();
	auto mediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
	auto primaryResource  = mMediaRef.getPropertyResource(mediaPropertyKey);

	if (primaryResource.getType() != ds::Resource::VIDEO_TYPE &&
		primaryResource.getType() != ds::Resource::YOUTUBE_TYPE) {
		return;
	}

	mvs.mVideoAutoPlayFirstFrame = false;
	mMediaPlayer->setSettings(mvs);

	auto theLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("thumb_hodler"); // TODO fix typo
	auto vidThumb  = mRootLayout->getSprite<ds::ui::Image>("video_thumb");
	if (theLayout && vidThumb) {
		theLayout->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [theLayout] { theLayout->hide(); });
		vidThumb->enable(false);
	}

	mMediaPlayer->loadMedia(primaryResource);
	auto wafflesHelper = ContentHelperFactory::getDefault<WafflesHelper>();
	if (wafflesHelper) {
		wafflesHelper->setMediaInterfaceStyle(mMediaPlayer->getMediaInterface());
	}
	// ContentUtils::setMediaInterfaceStyle(mMediaPlayer->getMediaInterface());
	mRootLayout->runLayout();
	mMediaPlayer->enter();
}

void TitledMediaViewer::processAllowedButtons() const {
	auto allowDrawing	 = mEngine.getWafflesSettings().getBool("media_viewer:allow_drawing", 0, true);
	auto allowRotation	 = mEngine.getWafflesSettings().getBool("media_viewer:allow_rotation", 0, true);
	auto allowDuplicate	 = mEngine.getWafflesSettings().getBool("media_viewer:allow_duplicate", 0, true);
	auto allowFullscreen = mEngine.getWafflesSettings().getBool("media_viewer:allow_fullscreen", 0, true);
	auto allowDetach	 = mEngine.getWafflesSettings().getBool("media_viewer:allow_detach", 0, true);

	auto drawingSpr = mRootLayout->getSprite("drawing.the_button");
	if (drawingSpr) {
		if (allowDrawing) {
			drawingSpr->show();
		} else {
			drawingSpr->hide();
		}
	}

	auto rotationSpr = mRootLayout->getSprite("rotation.the_button");
	if (rotationSpr) {
		if (allowRotation) {
			rotationSpr->show();
		} else {
			rotationSpr->hide();
		}
	}

	auto duplicateSpr = mRootLayout->getSprite("duplicate.the_button");
	if (duplicateSpr) {
		if (allowDuplicate) {
			duplicateSpr->show();
		} else {
			duplicateSpr->hide();
		}
	}

	auto fullscreenSpr = mRootLayout->getSprite("fullscreen.the_button");
	if (fullscreenSpr) {
		if (allowFullscreen && mCanFullscreen) {
			fullscreenSpr->show();
		} else {
			fullscreenSpr->hide();
		}
	}

	auto detachSpr = mRootLayout->getSprite("detach.the_button");
	if (detachSpr) {
		if (allowDetach && mCanDetach && mCanAttach) {
			detachSpr->show();
		} else {
			detachSpr->hide();
		}
	}

	mRootLayout->runLayout();
}

void TitledMediaViewer::processAllowedTouch() {
	// Enable/disable touch events but keep constraints.
	if (mIsFullscreen) {
		enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
	} else if (mIsDetached) {
		enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
	} else {
		disableMultiTouch();
	}
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
	if (mCreationArgs.mUseHotspots) {
		loadHotspots();
	}

	if (mRootLayout) {
		mRootLayout->completeAllTweens(false, true);
		mRootLayout->setSize(getWidth(), getHeight());

		mRootLayout->runLayout();
		mRootLayout->clearAnimateOnTargets(true);
	}
}

void TitledMediaViewer::loadHotspots() {
	if (mMediaRef.getPropertyBool("hide_controls")) {
		mEngine.timedCallback(
			[this]() {
				if (mMediaPlayer) {
					auto mediaInterface = mMediaPlayer->getMediaInterface();
					if (mediaInterface) {
						mediaInterface->setTransparent(true);
						mediaInterface->setOpacity(0);
						mediaInterface->enable(false);
						mediaInterface->hide();
						mediaInterface->setSize(1,1);
						mediaInterface->mLayoutSize = ci::vec2(1,1);
					}
				}
			},
			0.01f
		);
	}
	for (const auto& hs : mMediaRef.getChildren()) {
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
		hotspot->setTapCallback([this, hotspot](Sprite* bs, const ci::vec3& pos) {
			auto helper = ContentHelperFactory::getDefault<WafflesHelper>();
			auto field_name =
				mEngine.getWafflesSettings().getString("hotspot:destination:field_name", 0, "destination");
			auto destId = hotspot->getContentModel().getPropertyString(field_name);
			if (hotspot->getContentModel().getPropertyBool("trigger_ambient")) {
				mEngine.startIdling();
			} else if (!destId.empty()) {
				// launch the thing for the hotspot at pos
				ContentModelRef linkMedia = helper->getRecordByUid(destId);
				if (linkMedia.empty()) {
					DS_LOG_WARNING("Hotspot node not found for id == " << destId);
				} else {

					if (ContentUtils::getDefault(mEngine)->isMedia(linkMedia)) {
						mEventClient.notify(RequestViewerLaunchEvent(
							ViewerCreationArgs(linkMedia, VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
					} else {
						mEventClient.notify(RequestEngagePresentation(linkMedia));
					}
				}
			} else if (!hotspot->getContentModel().getChildren().empty()) {
				auto position = pos;
				for (const auto& child : hotspot->getContentModel().getChildren()) {
					mEventClient.notify(
						RequestViewerLaunchEvent(ViewerCreationArgs(child, VIEW_TYPE_TITLED_MEDIA_VIEWER, position)));
					position = position + mEngine.getWafflesSettings().getVec3(
											  "launcher:presentation:hotspot:asset:offset", 0, ci::vec3(45, 45, 0));
				}
			} else {
				// Not a valid hotspot
				DS_LOG_INFO("Invalid hotspot encountered! UID: " << destId);
			}
			// if (destId < 1) return; // nothing specified, skip it
		});

		if (mCreationArgs.mTouchEvents) {
			hotspot->setProcessTouchCallback([this](Sprite* bs, const ds::ui::TouchInfo& ti) {
				if (ti.mPhase == ds::ui::TouchInfo::Moved &&
					distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()) {
					bs->passTouchToSprite(this, ti);
					return;
				}
			});
		}

		// Layout the hotspot
		ci::vec2 lpos	= ci::vec2(hs.getPropertyFloat("hotspot_x"), hs.getPropertyFloat("hotspot_y"));
		ci::vec2 lsize	= ci::vec2(hs.getPropertyFloat("hotspot_w"), hs.getPropertyFloat("hotspot_h"));
		bool	 lnoXY	= (lpos.x == 0.f && lpos.y == 0.f);
		bool	 lnoWH	= (lsize.x == 0.f && lsize.y == 0.f);
		if (lnoXY && lnoWH) {
			lsize = ci::vec2(1.f);
		}
		hotspot->setPosition(lpos * ci::vec2(mMediaPlayer->getSize()));
		hotspot->setSize(lsize * ci::vec2(mMediaPlayer->getSize()));
		hotspot->runLayout();
	}

	//layoutHotspots();
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
				video->setLooping(mCreationArgs.mLooped);
				if (mCreationArgs.mVideoTimePosition > 0.0) {
					video->seekPosition(mCreationArgs.mVideoTimePosition);
				}
				video->setVolume(float(mCreationArgs.mVolume) / 100.0f);
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

		if (getIsDetached()) showTitle();
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

	processAllowedButtons();
	processAllowedTouch();
}

void TitledMediaViewer::onDetachedSet() {
	if (!mIsDetached) {
		hideTitle();
		// hideInnerSideBar();
	} else {
		showTitle();
		// showInnerSideBar();
	}
	processAllowedButtons();
	processAllowedTouch();
}

void TitledMediaViewer::toggleDrawing() {
	// if (!mShowingTitle && !mIsFullscreen) return;

	mDrawingMode = !mDrawingMode;

	if (mDrawingMode) {
		// DS_LOG_INFO("DRAWING ENABLED FOR VIEWER TYPE: " << this->getViewerType() <<
		// mMediaPlayer->getSpriteName().c_str());
		hideTitle();
		if (!mDrawingArea) {
			float widdy = mEngine.getAppSettings().getFloat("drawing:initial_resolution", 0, 3000.0f);
			widdy		= mEngine.getWafflesSettings().getFloat("drawing:initial_resolution", 0, widdy);

			float asp = getWidth() / getHeight();

			mDrawingArea = new DrawingArea(mEngine, widdy, widdy / asp, getChannelName());
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
			auto mediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
			if (mMediaRef.getPropertyResource(mediaPropertyKey).getType() != ds::Resource::VIDEO_STREAM_TYPE) {
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

void TitledMediaViewer::cleanupDrawing(bool clearDrawArea = false) {
	if (getIsDrawingMode()) {
		/// in some cases we might want to keep the current draw context alive
		/// but still hide it just in case with the toggle
		if (clearDrawArea) {
			mDrawingArea->clearAllDrawing();
		}
		toggleDrawing();
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

void TitledMediaViewer::mute() {
	if (mMediaPlayer) {
		mMediaPlayer->mute();
	}
}


void TitledMediaViewer::unmute() {
	if (mMediaPlayer) {
		mMediaPlayer->unmute();
	}
}

ViewerCreationArgs TitledMediaViewer::getDuplicateCreationArgs() const {

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
		args.mVolume			= int(roundf(theVideo->getVolume() * 100.0f));
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

void TitledMediaViewer::setInterfaceLocked(bool isLocked) const {
	if (!mMediaPlayer) return;

	if (auto web = dynamic_cast<ds::ui::WebInterface*>(mMediaPlayer->getMediaInterface())) {
		if (isLocked) {
			web->startTouch();
		} else {
			web->stopTouch();
		}
	} else if (auto pdf = dynamic_cast<ds::ui::PDFInterface*>(mMediaPlayer->getMediaInterface())) {
		if (isLocked) {
			pdf->startTouch();
		} else {
			pdf->stopTouch();
		}
	}
}

void TitledMediaViewer::rotateMedia() {
	if (!mMediaPlayer || mAnimationCount) return;

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
	mMediaPlayer->setRotation(ci::vec3(0.0f, 0.0f, 90.0f * float(mMediaRotation)));

	float w = mMediaPlayer->getHeight();
	float h = mMediaPlayer->getWidth();

	if (mMediaRotation % 2 == 0) {
		w = mMediaPlayer->getWidth();
		h = mMediaPlayer->getHeight();
	}

	mContentAspectRatio = w / h;

	setAbsoluteSizeLimits(ci::vec2(mMinSize.x, mMinSize.x / mContentAspectRatio),
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
	if (auto titleHolder = mRootLayout->getSprite("title_layout")) {
		titleHolder->show();
		titleHolder->tweenOpacity(1.0f, mEngine.getAnimDur());
	}
	onLayout();

	mControlsTimeoutTimer.timedCallback(
		[this] {
			if (mDrawingMode || mShowingKeyboard) return;
			hideTitle();
			hideInnerSideBar();
		},
		mEngine.getWafflesSettings().getDouble("media_viewer:control_timeout", 0, 5.0));
}

void TitledMediaViewer::hideTitle() {
	if (!mRootLayout || !mShowingTitle) return;
	mShowingTitle = false;
	if (auto titleHolder = mRootLayout->getSprite("title_layout")) {
		titleHolder->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone,
								  [titleHolder] { titleHolder->hide(); });
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

void TitledMediaViewer::setToFullscreen(const bool immediate, const bool showController) {

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
		BaseElement::setToFullscreen(immediate, showController);
	}
}

void TitledMediaViewer::checkBounds(bool immediate) {

	if (mPositionUpdateCallback) mPositionUpdateCallback();

	if (mAnimationCount && !immediate) return;


	// Constrain the bounding box of the sprite to mBoundingArea
	auto boundsMode = mIsFullscreen ? mFullscreenBoundsMode : mNormalBoundsMode;
	auto bb			= boundsMode == BoundsMode::kMediaEdge ? mMediaPlayer->getBoundingBox() : this->getBoundingBox();

	auto upperLeft	 = bb.getUpperLeft();
	auto bottomRight = bb.getLowerRight();

	if (boundsMode == BoundsMode::kMediaEdge) {
		const auto transform =
			glm::inverse(this->getParent()->getGlobalTransform()) * mMediaPlayer->getGlobalTransform();
		upperLeft	= transform * ci::vec4(bb.getX1(), bb.getY1(), 0, 1);
		bottomRight = transform * ci::vec4(bb.getSize(), 0, 1);
	}
	const float thisWidth  = bb.getWidth();
	const float thisHeight = bb.getHeight();
	const float thisX	   = upperLeft.x;
	const float thisY	   = upperLeft.y;

	// DS_LOG_INFO("BasePanel::checkBounds(): BB size: " << bb);

	const float worldL = mBoundingArea.getX1();
	const float worldR = mBoundingArea.getX2();
	const float worldW = mBoundingArea.getWidth();
	const float worldT = mBoundingArea.getY1();
	const float worldB = mBoundingArea.getY2();
	const float worldH = mBoundingArea.getHeight();

	float destinationX = thisX;
	float destinationY = thisY;

	if (thisWidth < worldW) {
		if (thisX < worldL) {
			destinationX = worldL;
		} else if (thisX > worldR - thisWidth) {
			destinationX = worldR - thisWidth;
		}
	} else {
		if (thisX < worldR - thisWidth) {
			destinationX = worldR - thisWidth;
		} else if (thisX > worldL) {
			destinationX = worldL;
		}
	}

	if (thisHeight < worldH) {
		if (thisY < worldT) {
			destinationY = worldT;
		} else if (thisY > worldB - thisHeight) {
			destinationY = worldB - thisHeight;
		}
	} else {
		if (thisY < worldB - thisHeight) {
			destinationY = worldB - thisHeight;
		} else if (thisY > worldT) {
			destinationY = worldT;
		}
	}

	auto pos = getPosition();

	if (!(destinationX == thisX && destinationY == thisY)) {
		mMomentum.deactivate();
	}

	// Compute the position of the upper-left corner of the rotated sprite, relative to the bounding box

	const auto normalizeAngle = [](const float degrees) {
		float ret = glm::mod(degrees, 360.0f);
		if (ret < 0) ret += 360.0f;
		return ret;
	};
	const float degrees = normalizeAngle(getRotation().z);

	const int	quadrant = (int)glm::floor(degrees / 90.0f);
	const float radians	 = glm::radians(degrees);
	const float w		 = boundsMode == BoundsMode::kMediaEdge ? mMediaPlayer->getScaleWidth() : getScaleWidth();
	const float h		 = boundsMode == BoundsMode::kMediaEdge ? mMediaPlayer->getScaleHeight() : getScaleHeight();
	const float W		 = bb.getWidth();
	const float H		 = bb.getHeight();
	const auto	ulPos =
		 (0 == quadrant) ? ci::vec2(h * glm::sin(radians), 0)
						 : ((1 == quadrant) ? ci::vec2(W, -h * glm::cos(radians))
											: ((2 == quadrant) ? ci::vec2(-w * glm::cos(radians), H) : //(3 == quadrant)
												   ci::vec2(0, -w * glm::sin(radians))));

	// DS_LOG_INFO("  BasePanel::checkBounds(): Constrained position: " << destinationX << ", " << destinationY <<
	// ", Angle: " << degrees << " degrees"  ); DS_LOG_INFO("  BasePanel::setBounds(): upper-left position: " <<
	// ulPos );

	// re-apply the anchor offset.
	const auto anchorOffset = ci::vec2(getCenter()) * ci::vec2(getScaleWidth(), getScaleHeight()) -
							  (boundsMode == BoundsMode::kMediaEdge ? ci::vec2(mLeftPad, mTopPad) : ci::vec2(0, 0));
	pos = ci::vec3(ci::vec2(destinationX, destinationY) + ulPos + glm::rotate(anchorOffset, radians), 0);


	if (immediate || (destinationX == thisX && destinationY == thisY)) {
		setPosition(pos);
	} else {
		tweenPosition(pos, mAnimDuration, 0.0f, ci::EaseOutQuint());
	}
}

void TitledMediaViewer::userInputReceived() {
	BasePanel::userInputReceived();

	// Input = focus = keyboard
	if (mMediaPlayer) {
		auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mMediaPlayer->getPlayer());
		if (webPlayer && webPlayer->getWeb()) {
			mEngine.registerEntryField(webPlayer->getWeb());
		}
	}

	if (mMediaPlayer && !mIsFullscreen && !mDrawingMode) {
		mMediaPlayer->showInterface();
	}

	layout();
}

void TitledMediaViewer::setKeyboardButtonImage(const std::string& imagePath, ds::ui::ImageButton* keyboardBtn) {
	keyboardBtn->setHighImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setNormalImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setColor(ci::Color::black());
	keyboardBtn->setCornerRadius(0.f);
}

} // namespace waffles
