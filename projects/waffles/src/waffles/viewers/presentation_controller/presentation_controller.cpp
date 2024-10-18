#include "stdafx.h"

#include "presentation_controller.h"


#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/media/interface/thumbnail_bar.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>
#include <ds/content/content_events.h>
#include "app/waffles_app_defs.h"
//#include "app/helpers.h"
#include <ds/ui/button/image_button.h>

#include "waffles/waffles_events.h"
#include "waffles/layer/template_layer.h"
#include <ds/app/engine/engine_events.h>


namespace waffles {

PresentationController::PresentationController(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mRootLayout(nullptr)
	, mShowingThumbs(false)
	, mShowingPins(false) {

	mCanResize			  = false;
	mCanFullscreen		  = false;
	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_PRESENTATION_CONTROLLER;
	mResetButtonEnabled	  = mEngine.getWafflesSettings().getBool("presentation_controller:reset_button:enabled", 0, true);

	mRootLayout = new ds::ui::SmartLayout(mEngine, "waffles/viewer/presentation_controller.xml");
	addChildPtr(mRootLayout);

	mRootLayout->setSpriteClickFn("end_presentation.the_button", [this] {
		//if (mEngine.mContent.getPropertyBool("presentation_controller_blocked")) return
		mEngine.notifyOnChannel(RequestPresentationEndEvent(), getChannelName());
		mEngine.notifyOnChannel(RequestCloseAllEvent(true), getChannelName());

		//mEngine.mContent.setProperty("presentation_controller_blocked", true);

		mCloseRequestCallback();
	});

	mRootLayout->setSpriteClickFn("refresh_layout.the_button", [this] {
		//if (mEngine.mContent.getPropertyBool("presentation_controller_blocked")) return;

		auto thePres = mEngine.mContent.getChildByName("current_presentation" + getChannelName());
		if (thePres.empty()) return;
		auto currentPresentation = thePres.getChildren().front();
		auto currentSlide		 = currentPresentation.getChildren()[thePres.getPropertyInt("current_slide") - 1];

		
		mEngine.notifyOnChannel(waffles::RequestCloseAllEvent(), getChannelName());
		mEngine.notifyOnChannel(waffles::RequestEngagePresentation(currentSlide), getChannelName());
	});

	mRootLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});

	mRootLayout->getSprite<ds::ui::ImageButton>("prev_step")->setTapCallback(
		[this] (ds::ui::Sprite* s, const ci::vec3& v) {
			mEngine.notifyOnChannel(waffles::RequestEngageBack(), getChannelName());
			updateUi();
		}
	);

	mRootLayout->getSprite<ds::ui::ImageButton>("next_step")->setTapCallback(
		[this] (ds::ui::Sprite* s, const ci::vec3& v) {
			mEngine.notifyOnChannel(waffles::RequestEngageNext(), getChannelName());
			updateUi();
		}
	);

	if (auto background = mRootLayout->getSprite("background")) {
		background->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
			if (ti.mPhase == ds::ui::TouchInfo::Moved && ti.mNumberFingers == 1) {
				bs->passTouchToSprite(this, ti);
				return;
			}
		});
	}

	if (!mResetButtonEnabled) {
		auto reset_button = mRootLayout->getSprite("refresh_layout.the_button");
		reset_button->hide();
		reset_button->setSize(ci::vec3(0));
		reset_button->enable(false);
	} // TODO: I'd prefer a release() call here and proper setting checking elsewhere, but cannot get working so here we are

	mRootLayout->runLayout();

	if (auto smarty = mRootLayout->getSprite<ds::ui::SmartScrollList>("thumb_scrolly")) {
		smarty->setContentItemTappedCallback([this](ds::ui::SmartLayout* item, ds::model::ContentModelRef model) {
			if (model != mCurrentSlide) {
				mEngine.notifyOnChannel(waffles::RequestEngagePresentation(model), getChannelName());
				updateUi();
			}
		});

		smarty->setContentItemUpdatedCallback([this](ds::ui::SmartLayout* item) {
			auto bg	   = item->getSprite("bg");
			auto label = item->getSprite("label");
			// auto icon  = item->getSprite<ds::ui::Image>("icon");
			if (!bg && !label /* && !icon*/) return;
			auto file = mEngine.getAppSettings().getString("template:icon:" +
														   item->getContentModel().getPropertyString("type_key"));
			// icon->setImageFile(ds::Environment::expand(file), ds::ui::Image::IMG_CACHE_F);
			// icon->setSize(180, 140);
			if (item->getContentModel() == mCurrentSlide) {
				bg->setColor(mEngine.getColors().getColorFromName("waffles:button:bg:high:dark"));
				label->setColor(mEngine.getColors().getColorFromName("waffles:button:text:high:dark"));
				// icon->setColor(mEngine.getColors().getColorFromName("white"));
			} else {
				bg->setColor(mEngine.getColors().getColorFromName("waffles:button:bg:normal:dark"));
				label->setColor(mEngine.getColors().getColorFromName("waffles:button:text:normal:dark"));
				// icon->setColor(mEngine.getColors().getColorFromName("white"));
			}
		});

		smarty->setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted) {
			ds::ui::SmartLayout* item = dynamic_cast<ds::ui::SmartLayout*>(bs);
			if (!item) return;
			auto bg	   = item->getSprite("bg");
			auto label = item->getSprite("label");
			// auto icon  = item->getSprite<ds::ui::Image>("icon");
			if (!bg && !label /* && !icon*/) return;

			if (highlighted || item->getContentModel() == mCurrentSlide) {
				bg->setColor(mEngine.getColors().getColorFromName("waffles:button:bg:high:dark"));
				label->setColor(mEngine.getColors().getColorFromName("waffles:button:text:high:dark"));
				// icon->setColor(mEngine.getColors().getColorFromName("white"));
			} else {
				bg->setColor(mEngine.getColors().getColorFromName("waffles:button:bg:normal:dark"));
				label->setColor(mEngine.getColors().getColorFromName("waffles:button:text:normal:dark"));
				// icon->setColor(mEngine.getColors().getColorFromName("white"));
			}
		});
	}

	mRootLayout->setSpriteClickFn("thumbs_toggle.the_button", [this] {
		//if (mEngine.mContent.getPropertyBool("presentation_controller_blocked")) return;
		toggleThumbs();
	});
	// mRootLayout->setSpriteClickFn("pins_toggle.the_button", [this] { togglePins(); });

	const float startWidth	= mRootLayout->getWidth();
	const float startHeight = mRootLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	BasePanel::setAbsoluteSizeLimits(ci::vec2(startWidth, startHeight), ci::vec2(startWidth, startHeight));

	setDefaultSize(ci::vec2(startWidth, startHeight));
	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);

	// mRootLayout->listenToEvents<ChangeTemplateRequest>([this](auto& e) { updateUi(); });
	auto uiUpdate = [this](auto& e) {
		callAfterDelay([this] { updateUi(); }, 0.01f);
	};
	mRootLayout->listenToEvents<waffles::ChangeTemplateRequest>(uiUpdate);

	mRootLayout->listenToEvents<RequestPinboardSave>([this](auto& e) { updatePins(); });

	// setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));


	// these are to hide this from showing up in saved drawings
	mEventClient.listenToEvents<RequestPreDrawingSave>([this](auto& e) { hide(); });
	mEventClient.listenToEvents<RequestDrawingSave>([this](auto& e) { show(); });
}

void PresentationController::onLayout() {
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}
}

void PresentationController::updateUi() {
	if (!mRootLayout) return;

	bool hide_slides = false; //mEngine.mContent.getPropertyBool("presentation_controller_blocked");

	auto thePres = mEngine.mContent.getChildByName("current_presentation" + getChannelName());
	auto currentPresentation =
		thePres.getChildren().empty() ? ds::model::ContentModelRef() : thePres.getChildren().front();
	auto currentSlide = currentPresentation.empty()
							? ds::model::ContentModelRef()
							: currentPresentation.getChildren()[thePres.getPropertyInt("current_slide") - 1];

	bool isNew		= false;
	auto prevPresId = mRootLayout->getContentModel().getPropertyString("uid");
	if (prevPresId != currentPresentation.getPropertyString("uid")) isNew = true;

	if (currentPresentation.empty()) {
		DS_LOG_WARNING("Current presentation is empty in presentation controller!");
	}

	if (isNew) {
		mRootLayout->setContentModel(currentPresentation);
	}

	if (hide_slides || currentPresentation.empty()) {
		mRootLayout->setSpriteText("slide_title", "Presentation Controller");
		mRootLayout->setSpriteText("name", "");
	} else {
		mRootLayout->setSpriteText("slide_title", currentSlide.getPropertyString("record_name"));
		if (thePres.getChildren().size() > 0)
			mRootLayout->setSpriteText("name", thePres.getChild(0).getPropertyString("display_name"));
	}

	int totalSlides = (int)currentPresentation.getChildren().size();
	int curSlide	= thePres.getPropertyInt("current_slide");

	if (hide_slides || currentPresentation.empty()) {
		mRootLayout->setSpriteText("slide_count", "No Active Presentation");
	} else {
		mRootLayout->setSpriteText("slide_count",
								   "Slide " + std::to_string(curSlide) + " of " + std::to_string(totalSlides));
	}

	mCurrentSlide = currentSlide;

	if (auto smarty = mRootLayout->getSprite<ds::ui::SmartScrollList>("thumb_scrolly")) {
		if (isNew) {
			smarty->setContentList(currentPresentation);
		} else {
			smarty->setContentListMaintainPosition(currentPresentation);
		}
		smarty->getScrollArea()->tweenScrollPercent(float(curSlide - 1) / float(totalSlides - 1));
	}

	bool isEnabled = true;
	if (currentPresentation.empty() || currentPresentation.getPropertyString("type_key") == "empty" ||
		currentPresentation.getPropertyString("type_key") == "pinboard_event" ||
		currentPresentation.getPropertyString("type_key") == "assets_mode") {
		isEnabled = false;
		if (mShowingThumbs) toggleThumbs();
	}
	for (auto& spName : {"next_step", "prev_step", "thumbs_toggle.the_button", "refresh_layout.the_button",
						 "end_presentation.the_button"}) {

		if (auto sp = mRootLayout->getSprite(spName)) {
			bool enabled = isEnabled && !hide_slides;
			sp->enable(isEnabled && !hide_slides);
			float opacity = enabled ? 1.f : 0.4f;
			if (((std::string)spName).find("step") != std::string::npos) {
				sp->setOpacity(opacity);
			} else {
				if (auto nl = mRootLayout->getSprite(ds::split(spName, ".")[0] + ".normal_label")) {
					nl->setOpacity(opacity);
				}
				if (auto ba = mRootLayout->getSprite(ds::split(spName, ".")[0] + ".backy")) {
					ba->setOpacity(opacity);
				}
			}
		}
	}

	layout();
}

void PresentationController::updatePins() {
	if (auto pinBar = mRootLayout->getSprite<ds::ui::ThumbnailBar>("pins")) {
		mPinMap.clear();
		auto					  theBoard = mEngine.mContent.getChildByName("current_pinboard");
		auto					  theItems = theBoard.getReferences("pinboard_items");
		std::vector<ds::Resource> pins;
		for (auto&& [key, item] : theItems) {
			auto theRes							   = item.getPropertyResource("media_thumb_res");
			mPinMap[item.getPropertyString("uid")] = theRes;
			pins.push_back(theRes);
		}

		ds::Resource parentResource;
		parentResource.setWidth(128.0f);
		parentResource.setHeight(92.0f);
		parentResource.setChildrenResources(pins);
		pinBar->setData(parentResource);
		pinBar->setHighlightedItem(-1);
	}
}


void PresentationController::toggleThumbs() {
	mShowingThumbs = !mShowingThumbs;


	auto thumbs		  = mRootLayout->getSprite("thumb_scrolly");
	auto thumbTog	  = mRootLayout->getSprite<ds::ui::LayoutButton>("thumbs_toggle.the_button");
	auto thumbsLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("thumbs_holder");
	if (!thumbsLayout || !thumbTog || !thumbs) {
		return;
	}

	ci::vec3 destSize;
	if (mShowingThumbs) {
		thumbTog->showDown();
		destSize = ci::vec3(mRootLayout->getWidth(), thumbs->getHeight(), 0.0f);
	} else {
		// thumbTog->showUp();
		destSize = ci::vec3(mRootLayout->getWidth(), 0.0f, 0.0f);
	}
	tweenStarted();
	if (!mShowingThumbs) thumbs->hide();
	thumbsLayout->tweenSize(
		destSize, mEngine.getAnimDur(), 0.0f, ci::easeInOutQuint,
		[this, thumbs] {
			tweenEnded();
			if (mShowingThumbs) thumbs->show();
		},
		[this] { reconfigureSize(); });
	updateUi();
}

void PresentationController::togglePins() {
	mShowingPins = !mShowingPins;

	auto pinsTog	= mRootLayout->getSprite<ds::ui::LayoutButton>("pins_toggle.the_button");
	auto pinsLayout = mRootLayout->getSprite<ds::ui::LayoutSprite>("pinboard_holder");
	if (!pinsTog || !pinsLayout) {
		return;
	}

	ci::vec3 destSize;
	if (mShowingPins) {
		pinsTog->showDown();
		destSize = ci::vec3(pinsLayout->getWidth(), 100.f, 0.0f);
		updatePins();
		mRootLayout->listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& e) { updatePins(); });
	} else {
		pinsTog->showUp();
		destSize = ci::vec3(pinsLayout->getWidth(), 0.0f, 0.0f);
		mRootLayout->stopListeningToEvents<ds::ScheduleUpdatedEvent>();
	}
	tweenStarted();
	pinsLayout->tweenSize(
		destSize, mEngine.getAnimDur(), 0.0f, ci::easeInOutQuint,
		[this] {
			tweenEnded();
			reconfigureSize();
		},
		[this] { reconfigureSize(); });
}


void PresentationController::onCreationArgsSet() {
	updateUi();
}

void PresentationController::reconfigureSize() {
	if (!mRootLayout) return;
	mRootLayout->runLayout();
	const float startWidth	= mRootLayout->getWidth();
	const float startHeight = mRootLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;
	setViewerSize(startWidth, startHeight);
	setSize(startWidth, startHeight);
}

} // namespace waffles
