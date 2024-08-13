#include "stdafx.h"

#include "ambient_chooser.h"

#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/control/control_slider.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>

#include <ds/ui/button/layout_button.h>

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {
AmbientChooser::AmbientChooser(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/settings/ambient_chooser.xml") {

	setSpriteClickFn("toggle.the_button", [this] {
		auto ambient = mEngine.mContent.getChildByName("ambient");
		ambient.setProperty("show_title", !ambient.getPropertyBool("show_title"));
		mEngine.getNotifier().notify(AmbientSettingsUpdated());
	});

	/* Should there even be defaults?
	setSpriteClickFn("default_image.the_button", [this] {
		auto& ambient = mEngine.mContent.getChildByName("ambient");
		auto userMedia = ambient.getChild("default_image");
		ambient.setPropertyResource("media_res", userMedia.getPropertyResource("media_res"));
		mEngine.getNotifier().notify(AmbientSettingsUpdated());
	});

	setSpriteClickFn("default_video.the_button", [this] {
	auto& ambient = mEngine.mContent.getChildByName("ambient");
	auto userMedia = ambient.getChild("default_video");
	ambient.setPropertyResource("media_res", userMedia.getPropertyResource("media_res"));
	mEngine.getNotifier().notify(AmbientSettingsUpdated());
	});

	*/

	setSpriteClickFn("user_media.the_button", [this] {
		auto ambient = mEngine.mContent.getChildByName("ambient");
		if (ambient.getPropertyInt("type") == AMBIENT_TYPE_USER_MEDIA) {
			ambient.setProperty("type", AMBIENT_TYPE_NONE);
		} else {
			ambient.setProperty("type", AMBIENT_TYPE_USER_MEDIA);
		}
		mEngine.getNotifier().notify(AmbientSettingsUpdated());
	});

	setSpriteClickFn("select_media.the_button", [this] {
		auto selectMediaButton = getSprite<ds::ui::LayoutButton>("select_media.the_button");
		if (selectMediaButton) {
			auto possy = selectMediaButton->getPosition();
			if (selectMediaButton->getParent()) {
				possy = selectMediaButton->getParent()->localToGlobal(getPosition());
			}
			ViewerCreationArgs vca =
				ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_SELECT_MEDIA_AMBIENT, possy);
			mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
		}
	});

	setSpriteClickFn("start_ambient.the_button", [this] { mEngine.startIdling(); });

	auto ambientTimeoutSlider = getSprite<ds::ui::ControlSlider>("timeout_slider");
	if (ambientTimeoutSlider) {
		auto nub = ambientTimeoutSlider->getNubSprite();
		if (nub) {
			nub->setColorA(mEngine.getColors().getColorFromName("ui_highlight"));
		}
		auto backy = ambientTimeoutSlider->getBackgroundSprite();
		if (backy) {
			backy->setColorA(mEngine.getColors().getColorFromName("ui_normal"));
		}

		ambientTimeoutSlider->setSliderInterpolation(ds::ui::ControlSlider::kSliderTypeQuadratic);
		ambientTimeoutSlider->setSliderLimits(5.0, 60.0 * 60.0 * 24.0); // one day

		ambientTimeoutSlider->setSliderUpdatedCallback(
			[this](const double percenty, const double valuey, const bool finished) {
				int theValue = (int)ds::math::round(valuey);
				mEngine.setIdleTimeout((int)theValue);

				if (finished) {
					mEngine.getNotifier().notify(AmbientSettingsUpdated());
				} else {
					updateButtons();
				}
			});

		ambientTimeoutSlider->setSliderValue((double)mEngine.getIdleTimeout());
	}

	auto slideSlider = getSprite<ds::ui::ControlSlider>("slide_slider");
	if (slideSlider) {
		auto nub = slideSlider->getNubSprite();
		if (nub) {
			nub->setColorA(mEngine.getColors().getColorFromName("ui_highlight"));
		}
		auto backy = slideSlider->getBackgroundSprite();
		if (backy) {
			backy->setColorA(mEngine.getColors().getColorFromName("ui_normal"));
		}

		slideSlider->setSliderInterpolation(ds::ui::ControlSlider::kSliderTypeQuadratic);
		slideSlider->setSliderLimits(5.0, 60.0 * 60.0); // one hour

		slideSlider->setSliderUpdatedCallback([this](const double percenty, const double valuey, const bool finished) {
			auto theAmbient = mEngine.mContent.getChildByName("ambient");
			theAmbient.setProperty("slide_seconds", valuey);

			if (finished) {
				mEngine.getNotifier().notify(AmbientSettingsUpdated());
			} else {
				updateButtons();
			}
		});

		auto theAmbient = mEngine.mContent.getChildByName("ambient");
		slideSlider->setSliderValue(theAmbient.getPropertyDouble("slide_seconds"));
	}


	mEventClient.listenToEvents<AmbientSettingsUpdated>([this](auto& e) { updateButtons(); });

	setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mLayoutUserType = kFlexSize;
	setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);

	updateButtons();
}

void AmbientChooser::updateButtons() {
	auto userMediaButton	= getSprite<ds::ui::LayoutButton>("user_media.the_button");
	auto defaultVideoButton = getSprite<ds::ui::LayoutButton>("default_video.the_button");
	auto defaultImageButton = getSprite<ds::ui::LayoutButton>("default_image.the_button");
	auto toggleUiButton		= getSprite<ds::ui::LayoutButton>("toggle.the_button");
	auto ambient			= mEngine.mContent.getChildByName("ambient");
	auto userAmbient		= ambient.getChild(0);

	if (toggleUiButton) {
		if (ambient.getPropertyBool("show_title")) {
			toggleUiButton->showDown();
		} else {
			toggleUiButton->showUp();
		}
	}

	if (userMediaButton) userMediaButton->showUp();
	if (defaultVideoButton) defaultVideoButton->showUp();
	if (defaultImageButton) defaultImageButton->showUp();


	std::string theTitle = userAmbient.getPropertyString("name");
	if (theTitle.empty()) {
		theTitle = "Custom Media - None Selected";
	}

	setSpriteText("user_media.label_high", theTitle);
	setSpriteText("user_media.label_normal", theTitle);
	if (userMediaButton) userMediaButton->runLayout();

	auto theType = ambient.getPropertyInt("type");
	switch (theType) {
	case AMBIENT_TYPE_NONE:
		break;
	case AMBIENT_TYPE_DEFAULT_IMAGE:
		if (defaultImageButton) defaultImageButton->showDown();
		break;
	case AMBIENT_TYPE_DEFAULT_VIDEO:
		if (defaultVideoButton) defaultVideoButton->showDown();
		break;
	case AMBIENT_TYPE_USER_MEDIA:
		if (userMediaButton) userMediaButton->showDown();
		break;
	}


	auto timeoutTimeText = getSprite<ds::ui::Text>("timeout_time");
	if (timeoutTimeText) {
		int theValue = mEngine.getIdleTimeout();
		if (theValue < 60) {
			timeoutTimeText->setText(std::to_string(theValue) + " Seconds");
		} else if (theValue < 60 * 60) {
			timeoutTimeText->setText(std::to_string(theValue / 60) + " Minutes");
		} else {
			timeoutTimeText->setText(std::to_string(theValue / (60 * 60)) + " Hours");
		}
	}


	auto slideTimeText = getSprite<ds::ui::Text>("slide_time");
	if (slideTimeText) {
		int theValue = (int)ds::math::round(ambient.getPropertyInt("slide_seconds"));
		if (theValue < 60) {
			slideTimeText->setText(std::to_string(theValue) + " Seconds");
		} else if (theValue < 60 * 60) {
			slideTimeText->setText(std::to_string(theValue / 60) + " Minutes");
		} else {
			slideTimeText->setText(std::to_string(theValue / (60 * 60)) + " Hours");
		}
	}
}
} // namespace waffles
