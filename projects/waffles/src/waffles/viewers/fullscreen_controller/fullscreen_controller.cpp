#include "stdafx.h"

#include "fullscreen_controller.h"

#include <ds/app/environment.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/media_interface_builder.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"
#include "waffles/common/ui_utils.h"
#include "waffles/pinboard/pinboard_button.h"
#include "waffles/viewers/drawing/drawing_area.h"
#include "waffles/viewers/drawing/drawing_tools.h"
#include "waffles/viewers/titled_media_viewer.h"
#include "waffles/viewers/viewer_controller.h"

namespace waffles {

FullscreenController::FullscreenController(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mEventClient(g)
	, mRootLayout(nullptr)
	, mMediaInterface(nullptr)
	, mDrawingTools(nullptr)
	, mLinkedMediaViewer(nullptr) {

	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_FULLSCREEN_CONTROLLER;

	mRootLayout = new ds::ui::SmartLayout(mEngine, "waffles/viewer/fullscreen_controller.xml");
	addChildPtr(mRootLayout);


	mRootLayout->setSpriteClickFn("close_button.the_button", [this] {
		// if you close the controlelr while drawing, it's possible to not be able to bring the FSC back up
		if (mLinkedMediaViewer && mLinkedMediaViewer->getIsDrawingMode()) return;
		if (mMediaInterface && mMediaInterface->isLocked()) {
			return;
		}

		removeDrawingTools();

		if (mCloseRequestCallback) mCloseRequestCallback();
	});


	mRootLayout->setSpriteClickFn("fullscreen.the_button", [this] {
		if (mLinkedMediaViewer) {
			if (mLinkedMediaViewer->getIsFullscreen()) {
				mEngine.getNotifier().notify(RequestUnFullscreenViewer(mLinkedMediaViewer));
			} else {
				ci::vec3 pos = getPosition();
				mEngine.getNotifier().notify(RequestFullscreenViewer(mLinkedMediaViewer));
				// immediately send a request for this viewer, otherwise it gets sent to the center of the screen by
				// default
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(
					ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_FULLSCREEN_CONTROLLER, pos,
									   ViewerCreationArgs::kViewLayerTop, 0.0f, false)));
			}
		}
	});


	mRootLayout->setSpriteClickFn("drawing.the_button", [this] {
		if (mLinkedMediaViewer) {
			mLinkedMediaViewer->toggleDrawing();
			setDrawingToolsState();
		}
	});

	mEventClient.listenToEvents<ViewerRemovedEvent>([this](auto& e) {
		if (e.mViewer == mLinkedMediaViewer) {
			linkMediaViewer(nullptr);
		}
	});
	//	mEventClient.listenToEvents<ViewerUpdatedEvent>([this](auto& e) { updateUi(); });

	// these are to hide this from showing up in saved drawings
	mEventClient.listenToEvents<RequestPreDrawingSave>([this](auto& e) { hide(); });
	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) { show(); });


	mRootLayout->runLayout();
	const float startWidth	= mRootLayout->getWidth();
	const float startHeight = mRootLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	BasePanel::setAbsoluteSizeLimits(ci::vec2(startWidth, startHeight), ci::vec2(startWidth, startHeight));

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);

	auto vc = ViewerController::getInstance();
	if (vc) {
		auto tmvs = vc->getViewersOfType(VIEW_TYPE_TITLED_MEDIA_VIEWER);
		if (!tmvs.empty()) {
			linkMediaViewer(dynamic_cast<TitledMediaViewer*>(tmvs.back()));
		}
	}

	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void FullscreenController::linkMediaViewer(TitledMediaViewer* tmv) {
	if (tmv == mLinkedMediaViewer) return;
	mLinkedMediaViewer = tmv;

	if (tmv != nullptr) {
		if (auto pb = mRootLayout->getSprite<PinboardButton>("pinboard")) {
			auto med = mLinkedMediaViewer->getMedia();
			pb->setContentModel(med);
		}
	}
	
	updateUi();
}

void FullscreenController::onLayout() {
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}

	if (mDrawingTools) {
		mDrawingTools->setSize(getWidth(), mDrawingTools->getHeight());
		mDrawingTools->setPosition(0.0f, getHeight()-8); //stupid thing shoved up so you cant see the janky
	}
}

void FullscreenController::updateUi() {
	if (!mRootLayout) return;

	if (mMediaInterface) {
		mMediaInterface->release();
		mMediaInterface = nullptr;
	}
	auto interfaceHolder = mRootLayout->getSprite("controller_holder");
	if (mLinkedMediaViewer && interfaceHolder) {
		auto contentRef	 = mLinkedMediaViewer->getMedia();
		auto mediaPlayer = mLinkedMediaViewer->getMediaPlayer();
		mRootLayout->setContentModel(contentRef);

		if (mediaPlayer && mediaPlayer->getPlayer()) {
			mMediaInterface =
				ds::ui::MediaInterfaceBuilder::buildMediaInterface(mEngine, mediaPlayer->getPlayer(), interfaceHolder);

			setMediaInterfaceStyle(mMediaInterface);

			if (mMediaInterface) {
				mMediaInterface->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;

				if (auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaInterface)) {
					webInterface->setKeyboardDisablesTimeout(false);
					ds::ui::ImageButton* keyboardBtn = webInterface->getKeyboardButton();
					webInterface->setKeyboardStateCallback([this, webInterface, keyboardBtn](const bool onScreen) {
						if (onScreen) {
							auto	  keeb			= webInterface->getSoftKeyboard();
							auto&	  setty			= keeb->getSoftKeyboardSettings();
							ci::Color lightGrey		= mEngine.getColors().getColorFromName("ui_icon_background");


							setty.mKeyDownColor		= ci::Color::black();
							setty.mKeyUpColor		= ci::Color(lightGrey);
							setty.mGraphicType		= ds::ui::SoftKeyboardSettings::kSolid;
							setty.mGraphicRoundedCornerRadius = 0;
							keeb->setSoftKeyboardSettings(setty);

							setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard on_64.png", keyboardBtn);

						} else if (!onScreen) {
							setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard_64.png", keyboardBtn);
						}
					});
				}

				mMediaInterface->setCanTimeout(false);

				// Handle lock state changes
				mMediaInterface->setLockStateCallback([this](bool lock) { updateLockedState(); });

				// Make sure we have the correct lock state right away too
				updateLockedState();
			}
		}

		setDrawingToolsState();

	} else {
		removeDrawingTools();
	}

	layout();
}

void FullscreenController::updateLockedState() {
	bool isLocked = (mMediaInterface && mMediaInterface->isLocked() );
	bool isDrawing = (mLinkedMediaViewer && mLinkedMediaViewer->getIsDrawingMode());

	if (auto closeBtn = mRootLayout->getSprite("close_button.the_button")) {
		if (isLocked || isDrawing) {
			closeBtn->enable(false);
			closeBtn->setOpacity(0.5f);
		} else {
			closeBtn->enable(true);
			closeBtn->setOpacity(1.0f);
		}
	}
}

void FullscreenController::setDrawingToolsState() {
	if (!mLinkedMediaViewer || !mRootLayout) return;

	auto drawingButt = mRootLayout->getSprite<ds::ui::LayoutButton>("drawing.the_button");
	if (!mLinkedMediaViewer->getIsDrawingMode()) {
		if (drawingButt) drawingButt->showUp();
		if (mDrawingTools) {
			mDrawingTools->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [this] {
				if (mDrawingTools) mDrawingTools->hide();
			});
		}

	} else {
		if (drawingButt) drawingButt->showDown();

		if (!mDrawingTools && mLinkedMediaViewer->getDrawingArea()) {
			mDrawingTools = mLinkedMediaViewer->getDrawingArea()->getDrawingTools();
			mLinkedMediaViewer->getDrawingArea()->setToolsEmbedded(false);
			if (mDrawingTools && mDrawingTools->getParent() != this) {
				addChildPtr(mDrawingTools);
			}
		}

		if (mDrawingTools) {
			mDrawingTools->show();
			mDrawingTools->tweenOpacity(1.0f, mEngine.getAnimDur(), 0.0f);
		}

		layout();
	}

	updateLockedState();
}

void FullscreenController::removeDrawingTools() {
	if (mLinkedMediaViewer && mDrawingTools && mLinkedMediaViewer->getDrawingArea()) {
		mLinkedMediaViewer->getDrawingArea()->setToolsEmbedded(true);
		mDrawingTools = nullptr;
	} else if (mDrawingTools) {
		mDrawingTools->release();
		mDrawingTools = nullptr;
	}
}

void FullscreenController::onAboutToBeRemoved() {
	removeDrawingTools();

	if (mMediaInterface) {
		auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaInterface);
		if (webInterface) {
			webInterface->linkWeb(nullptr);
		}
	}
}

void FullscreenController::setKeyboardButtonImage(std::string imagePath, ds::ui::ImageButton* keyboardBtn) {
	keyboardBtn->setHighImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setNormalImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setColor(ci::Color::black());
	keyboardBtn->setCornerRadius(0.f);
}

} // namespace waffles
