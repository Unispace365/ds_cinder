#include "stdafx.h"

#include "framed_fullscreen_controller.h"

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
#include <ds/ui/sprite/web.h>

#include "app/waffles_app_defs.h"
#include "waffles/common/ui_utils.h"
#include "waffles/pinboard/pinboard_button.h"
#include "waffles/viewers/drawing/drawing_area.h"
#include "waffles/viewers/drawing/drawing_tools.h"
#include "waffles/viewers/titled_media_viewer.h"
#include "waffles/viewers/viewer_controller.h"
#include "waffles/waffles_events.h"


namespace waffles {

FramedFullscreenController::FramedFullscreenController(ds::ui::SpriteEngine& g, const std::string layout)
  : BaseElement(g)
  , mLayoutFile(layout) {}

void FramedFullscreenController::linkMediaViewer(TitledMediaViewer* tmv) {
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

void FramedFullscreenController::init() {
	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_FULLSCREEN_CONTROLLER;

	mRootLayout = new ds::ui::SmartLayout(mEngine, mLayoutFile);
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

	mRootLayout->setSpriteClickFn("item_close_button.the_button", [this] {
		if (mLinkedMediaViewer) {
			mLinkedMediaViewer->close();
		}
	});

	mRootLayout->setSpriteClickFn("item_close_button.the_button", [this] {
		if (mLinkedMediaViewer) {
			mLinkedMediaViewer->close();
		}
	});

	auto collapseBtn = mRootLayout->getSprite<ds::ui::LayoutButton>("controller_collapse_btn.the_button");

	if (collapseBtn) {
		// Horrible little trick to ensure all the button states are configured
		collapse();
		uncollapse();

		// Handle button tapping + still allow a drag to start
		collapseBtn->setProcessTouchCallback([this, collapseBtn](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) {
			if (ti.mPhase == ds::ui::TouchInfo::Added) {
				// When touch is first added, show the down state
				collapseBtn->showDown();
			} else if (ti.mPhase == ds::ui::TouchInfo::Moved && !collapseBtn->contains(ti.mCurrentGlobalPoint)) {
				// Moved outside the button, now we want to pass back to our parent and hide the downstate
				// Since the parent only cares about moves this is all we need
				collapseBtn->showUp();
				sp->passTouchToSprite(this, ti);
			} else if (ti.mPhase == ds::ui::TouchInfo::Removed && collapseBtn->contains(ti.mCurrentGlobalPoint)) {
				// Finally, if we get the removed before we've passed, we want to collapse/uncollapse and set the button
				// back to the normal state
				collapseBtn->showUp();
				if (mLinkedMediaViewer && mLinkedMediaViewer->getIsDrawingMode()) return;
				if (mMediaInterface && mMediaInterface->isLocked()) {
					return;
				}
				removeDrawingTools();
				mIsCollapsed ? uncollapse() : collapse();
			}
		});
	}


	mRootLayout->setSpriteClickFn("fullscreen.the_button", [this] {
		if (mLinkedMediaViewer) {
			if (mLinkedMediaViewer->getIsFullscreen()) {
				mEventClient.notify(RequestUnFullscreenViewer(mLinkedMediaViewer));
			} else {
				ci::vec3 pos = getPosition();
				mEventClient.notify(RequestFullscreenViewer(mLinkedMediaViewer));
				// immediately send a request for this viewer, otherwise it gets sent to the center of the screen by
				// default
				mEventClient.notify(RequestViewerLaunchEvent(
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

	mEventClient.listenToEvents<RequestToggleCollapseAndMoveFullscreenController>([this](auto& e) {
		if (e.mShouldMove) {
			mIsCollapsed ? uncollapseAndMove(e.mPos) : collapseAndMove(e.mPos);
		} else {
			mIsCollapsed ? uncollapse() : collapse();
		}
	});

	mEventClient.listenToEvents<RequestCollapseAndMoveFullscreenController>([this](auto& e) {
		if (e.mShouldMove) {
			collapseAndMove(e.mPos);
		} else {
			collapse();
		}
	});

	mEventClient.listenToEvents<RequestUncollapseAndMoveFullscreenController>([this](auto& e) {
		if (e.mShouldMove) {
			uncollapseAndMove(e.mPos);
		} else {
			uncollapse();
		}
	});

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

	auto vc = ViewerControllerFactory::getInstanceOf(ci::vec2(0, 0), getChannelName());

	if (vc) {
		auto tmvs = vc->getViewersOfType(VIEW_TYPE_TITLED_MEDIA_VIEWER);
		if (!tmvs.empty()) {
			linkMediaViewer(dynamic_cast<TitledMediaViewer*>(tmvs.back()));
		}
	}

	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void FramedFullscreenController::onLayout() {
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}

	if (mDrawingTools) {
		mDrawingTools->setSize(getWidth(), mDrawingTools->getHeight());
		mDrawingTools->setPosition(0.0f, getHeight() - 8); // stupid thing shoved up so you cant see the janky
	}
}

void FramedFullscreenController::updateUi() {
	if (!mRootLayout) return;

	if (mMediaInterface) {
		mMediaInterface->release();
		mMediaInterface = nullptr;
	}
	auto interfaceHolder = mRootLayout->getSprite("controller_holder");

	if (mLinkedMediaViewer && interfaceHolder) {
		interfaceHolder->show();
		auto contentRef	 = mLinkedMediaViewer->getMedia();
		auto mediaPlayer = mLinkedMediaViewer->getMediaPlayer();
		mRootLayout->setContentModel(contentRef);

		if (mediaPlayer && mediaPlayer->getPlayer()) {
			mMediaInterface =
				ds::ui::MediaInterfaceBuilder::buildMediaInterface(mEngine, mediaPlayer->getPlayer(), interfaceHolder);

			auto wafflesHelper = ds::model::ContentHelperFactory::getDefault<waffles::WafflesHelper>();
			if (wafflesHelper) {
				wafflesHelper->setMediaInterfaceStyle(mMediaInterface);
			}
			// ContentUtils::setMediaInterfaceStyle(mMediaInterface);

			if (mMediaInterface) {
				mMediaInterface->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;

				if (auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaInterface)) {
					// webInterface->setKeyboardKeyScale(30.0f / 64.0f);
					webInterface->setKeyboardDisablesTimeout(false);
					webInterface->setKeyboardAbove(false);
					webInterface->setKeyboardOnTop(true);
					ds::ui::ImageButton* keyboardBtn = webInterface->getKeyboardButton();
					webInterface->setKeyboardStateCallback([this, webInterface, keyboardBtn](const bool onScreen) {
						if (onScreen) {
							auto	   keeb	 = webInterface->getSoftKeyboard();
							auto&	   setty = keeb->getSoftKeyboardSettings();
							ci::ColorA up	 = mEngine.getColors().getColorFromName("waffles_key_up");
							ci::ColorA down	 = mEngine.getColors().getColorFromName("waffles_key_down");
							ci::ColorA keyb	 = mEngine.getColors().getColorFromName("viewer_background");

							setty.mKeyDownColor				  = down;
							setty.mKeyUpColor				  = up;
							auto tc							  = setty.mKeyDnTextConfig;
							setty.mGraphicType				  = ds::ui::SoftKeyboardSettings::kSolid;
							setty.mGraphicRoundedCornerRadius = 8;
							setty.mGraphicKeySize			  = 30;
							keeb->setSoftKeyboardSettings(setty);

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
							// setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard on_64.png",
							//					   keyboardBtn);

						} else if (!onScreen) {

							auto normalColor = mEngine.getColors().getColorFromName("ui_normal");
							auto highColor	 = mEngine.getColors().getColorFromName("ui_selected");

							webInterface->getKeyboardButton()->setNormalImageColor(normalColor);
							webInterface->getKeyboardButton()->setHighImageColor(highColor);
							// setKeyboardButtonImage("%APP%/data/images/waffles/icons/1x/Keyboard_64.png",
							// keyboardBtn);
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
	mRootLayout->runLayout();
	layout();
}

void FramedFullscreenController::updateLockedState() {
	bool isLocked  = (mMediaInterface && mMediaInterface->isLocked());
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

void FramedFullscreenController::setDrawingToolsState() {
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

void FramedFullscreenController::removeDrawingTools() {
	if (mLinkedMediaViewer && mDrawingTools && mLinkedMediaViewer->getDrawingArea()) {
		mLinkedMediaViewer->getDrawingArea()->setToolsEmbedded(true);
		mDrawingTools = nullptr;
	} else if (mDrawingTools) {
		mDrawingTools->release();
		mDrawingTools = nullptr;
	}
}

void FramedFullscreenController::onAboutToBeRemoved() {
	removeDrawingTools();

	if (mMediaInterface) {
		auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mMediaInterface);
		if (webInterface) {
			webInterface->linkWeb(nullptr);
		}
	}
}

void FramedFullscreenController::onParentSet() {
	BaseElement::onParentSet();
	callAfterDelay([this] { init(); }, 0.1f);
}

void FramedFullscreenController::setKeyboardButtonImage(std::string imagePath, ds::ui::ImageButton* keyboardBtn) {
	keyboardBtn->setHighImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setNormalImage(ds::Environment::expand(imagePath), ds::ui::Image::IMG_CACHE_F);
	keyboardBtn->setColor(ci::Color::black());
	keyboardBtn->setCornerRadius(0.f);
}

void FramedFullscreenController::collapseAndMove(ci::vec3 pos) {
	collapse();
	auto parent = getParent();
	if (parent) {
		tweenPosition(parent->globalToLocal(pos), 0.25, 0.0, ci::easeOutCubic);
	}
}

void FramedFullscreenController::uncollapseAndMove(ci::vec3 pos) {
	uncollapse();
	auto parent = getParent();
	if (parent) {
		tweenPosition(parent->globalToLocal(pos), 0.25, 0.0, ci::easeOutCubic);
	}
}


void FramedFullscreenController::collapse() {
	if (mIsCollapsed) return;

	if (mLinkedMediaViewer && mLinkedMediaViewer->getIsDrawingMode()) return;
	if (mMediaInterface && mMediaInterface->isLocked()) {
		return;
	}

	removeDrawingTools();

	auto controls	 = mRootLayout->getSprite("border_layout");
	auto backRect	 = mRootLayout->getSprite("bg_filler");
	auto btnLayout	 = mRootLayout->getSprite<ds::ui::LayoutButton>("controller_collapse_btn.the_button");
	auto btn		 = mRootLayout->getSprite<ds::ui::Image>("controller_collapse_btn.icon");
	auto btnHigh	 = mRootLayout->getSprite<ds::ui::Image>("controller_collapse_btn.icon_high");
	auto normalColor = mEngine.getColors().getColorFromName("ui_normal");
	auto highColor	 = mEngine.getColors().getColorFromName("ui_selected");
	if (btn && btnHigh && btnLayout) {

		btn->setImageFile("%APP%/data/images/waffles/icons(framed)/expand=active2x.png");
		btnHigh->setImageFile("%APP%/data/images/waffles/icons(framed)/collapse2=active2x.png");
		btnLayout->runLayout();
	}

	// Only set the uncollapsed size once, otherwise repeated taps of the collapse button will use mid-tween values
	// breaking the layout
	if (!mUncollapsedSizeSet) mUncollapsedSize = backRect->getSize();

	if (controls && backRect) {
		controls->tweenOpacity(0, 0.25);
		backRect->tweenSize(ci::vec3(0, 0, 0), 0.25, 0.20);
	}
	mIsCollapsed = true;
}

void FramedFullscreenController::uncollapse() {
	if (!mIsCollapsed) return;
	auto controls	 = mRootLayout->getSprite("border_layout");
	auto backRect	 = mRootLayout->getSprite("bg_filler");
	auto normalColor = mEngine.getColors().getColorFromName("ui_normal");
	auto highColor	 = mEngine.getColors().getColorFromName("ui_selected");

	if (controls && backRect) {
		controls->tweenOpacity(1, 0.25, 0.20);
		backRect->tweenSize(mUncollapsedSize, 0.25);
	}
	auto btnLayout = mRootLayout->getSprite<ds::ui::LayoutButton>("controller_collapse_btn.the_button");
	auto btn	   = mRootLayout->getSprite<ds::ui::Image>("controller_collapse_btn.icon");
	auto btnHigh   = mRootLayout->getSprite<ds::ui::Image>("controller_collapse_btn.icon_high");
	if (btn && btnHigh && btnLayout) {

		btnHigh->setImageFile("%APP%/data/images/waffles/icons(framed)/expand=active2x.png");
		btn->setImageFile("%APP%/data/images/waffles/icons(framed)/collapse2=active2x.png");
		btnLayout->runLayout();
	}
	mIsCollapsed = false;
}

void FramedFullscreenController::onUpdateServer(const ds::UpdateParams& p) {
	BaseElement::onUpdateServer(p);
	// set our name to our mediaViewers name
	if (mLinkedMediaViewer && !mIsCollapsed) {
		auto mediaPlayer = mLinkedMediaViewer->getMediaPlayer();
		if (mediaPlayer) {
			auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mediaPlayer->getPlayer());
			if (webPlayer) {
				if (auto webby = webPlayer->getWeb()) {				
					mRootLayout->setSpriteText("title", webby->getPageTitle());
				}
			}
		}
	}
}

} // namespace waffles
