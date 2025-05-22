#include "stdafx.h"

#include "base_waffles_helper.h"

#include <ds/content/platform.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/toggle_container.h>
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/media/media_interface.h>


namespace waffles {

BaseWafflesHelper::BaseWafflesHelper(ds::ui::SpriteEngine& eng)
  : WafflesHelper(eng) {
}


bool BaseWafflesHelper::getApplyParticles() {
	auto platform = getPlatformModel();
	if (platform.empty()) return ContentModelRef();

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "scheduled_content_event" && !event.getProperty("particle_effect").empty()) {

				return event.getPropertyBool("particle_effect");
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled particle effect toggle for platform" << platform.getPropertyString("name"))
	}

	if (!platform.getProperty("particle_effect").empty()) {
		return platform.getPropertyBool("particle_effect");
	}

	DS_LOG_VERBOSE(1, "No platform particle effect toggle for platform" << platform.getPropertyString("name"))

	return false;
}


ContentModelRef BaseWafflesHelper::getPinboard() {
	auto pinboards = getValidPinboards();
	if (pinboards.empty()) {
		return {};
	}
	return pinboards.front();
}


std::vector<ContentModelRef> BaseWafflesHelper::getValidPinboards() {
	auto platform = getPlatformModel();
	if (platform.empty()) return {};

	std::vector<ContentModelRef> pinboards;

	// get all the events scheduled for this platform
	auto allPlatformEvents = platform.getChildByName("current_events").getChildren();
	for (const auto& event : allPlatformEvents) {
		if (event.getPropertyString("type_key") == "pinboard_event") {
			pinboards.push_back(event);
		}
	}
	if (pinboards.empty()) {
		pinboards.emplace_back();
		// DS_LOG_VERBOSE(1, "No scheduled pinboard for platform" << myPlatform.getPropertyString("name"))
	}
	return pinboards;
}


ContentModelRef BaseWafflesHelper::getAnnotationFolder() {
	ContentModelRef result;

	if (mAnnotationFolderKeys.empty()) {
		loadIntegration();
	}
	
	for (const auto& record : mEngine.mContent.getChildByName(ALL_RECORDS).getChildren()) {
		auto type  = record.getPropertyString("type_key");
		auto valid = std::find(mAnnotationFolderKeys.begin(), mAnnotationFolderKeys.end(), type) != mAnnotationFolderKeys.end();
		if (valid) {
			if (!record.empty()) {
				return record;
			}
		}
	}

	return result;
}

void BaseWafflesHelper::setKeyboardStyle(ds::ui::SoftKeyboard* keeb) {
	// Note, this only somewhat works. If the keyboard is initialized with the wrong key size this will resize the keys
	// but not correctly re-size the keyboard. Leading to either overlaps or huge gaps
	// More refinement needed
	auto& setty = keeb->getSoftKeyboardSettings();

	ci::ColorA up	= mEngine.getColors().getColorFromName("waffles_key_up");
	ci::ColorA down = mEngine.getColors().getColorFromName("waffles_key_down");

	setty.mKeyDownColor				  = down;
	setty.mKeyUpColor				  = up;
	auto tc							  = setty.mKeyDnTextConfig;
	setty.mGraphicType				  = ds::ui::SoftKeyboardSettings::kSolid;
	setty.mGraphicRoundedCornerRadius = 8;
	setty.mGraphicKeySize			  = 30;

	keeb->setSoftKeyboardSettings(setty);
}

void BaseWafflesHelper::setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) {
	if (!interfacey) return;
	auto& mEngine = interfacey->getEngine();

	auto cornerRad		= mEngine.getWafflesSettings().getFloat("ui:corner_radius", 0, 0.0f);
	auto interfaceScale = mEngine.getWafflesSettings().getFloat("ui:interface_scale", 0, 1.0f);

	auto viewerBackground = mEngine.getColors().getColorFromName("viewer_background");
	auto backgroundColor  = mEngine.getColors().getColorFromName("ui_background");
	auto normalColor	  = mEngine.getColors().getColorFromName("ui_normal");
	auto highColor		  = mEngine.getColors().getColorFromName("waffles_bloom");

	constexpr auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

	if (auto vidInterface = dynamic_cast<ds::ui::VideoInterface*>(interfacey)) {
		auto interfaceHeight = vidInterface->getHeight();
		if (auto play = vidInterface->getPlayButton()) {
			vidInterface->setButtonColor(play, normalColor, highColor);
		}

		if (auto pause = vidInterface->getPauseButton()) {
			vidInterface->setButtonColor(pause, normalColor, highColor);
		}

		if (auto loopy = vidInterface->getLoopButton()) {
			vidInterface->setButtonColor(loopy, normalColor, highColor);
		}

		if (auto unloopy = vidInterface->getUnLoopButton()) {
			vidInterface->setButtonColor(unloopy, normalColor, highColor);
		}

		if (vidInterface->getScrubBarBackground() && vidInterface->getScrubBarProgress()) {
			vidInterface->getScrubBarBackground()->setColor(normalColor);
			vidInterface->getScrubBarBackground()->setOpacity(0.2);
			vidInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			vidInterface->getScrubBarProgress()->setColor(normalColor);
			vidInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (vidInterface->getVolumeControl()) {
			auto volumeControl = vidInterface->getVolumeControl();
			volumeControl->setMuteImage("%APP%/data/images/waffles/icons/4x/Mute_256.png");
			volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons/4x/Volume low_256.png");
			volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons/4x/Volume high_256.png");
			volumeControl->setSliderHeight(8.f);
			volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			vidInterface->setButtonColor(sliderSprites.mMuteButton, normalColor, highColor);
			vidInterface->setButtonColor(sliderSprites.mVolHighButton, normalColor, highColor);
			vidInterface->setButtonColor(sliderSprites.mVolLowButton, normalColor, highColor);

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.2);
			sliderSprites.mSliderTrack->setCornerRadius(cornerRad);

			sliderSprites.mSliderFill->setColor(normalColor);
			sliderSprites.mSliderFill->setCornerRadius(cornerRad);

			sliderSprites.mSliderNub->setColor(normalColor);
			sliderSprites.mSliderNub->setScale(sliderSprites.mSliderNub->getScale() * 1.25f);
			sliderSprites.mSliderNub->setCornerRadius(100.f);

			/* volumeControl->setStyle(ds::ui::VideoVolumeStyle::CLASSIC);
			auto volumeBars = vidInterface->getVolumeControl()->getBars();
			for (auto eachBar : volumeBars) {
				eachBar->setColor(normalColor);
				eachBar->setCornerRadius(cornerRad);
			}
			*/
		}
	}

	if (auto ytInterface = dynamic_cast<ds::ui::YoutubeInterface*>(interfacey)) {
		auto interfaceHeight = ytInterface->getHeight();
		if (auto play = ytInterface->getPlayButton()) {
			ytInterface->setButtonColor(play, normalColor, highColor); // play button
		}

		if (auto pause = ytInterface->getPauseButton()) {
			ytInterface->setButtonColor(pause, normalColor, highColor);
		}

		if (ytInterface->getScrubBarBackground() && ytInterface->getScrubBarProgress()) {
			ytInterface->getScrubBarBackground()->setColor(normalColor);
			ytInterface->getScrubBarBackground()->setOpacity(0.2);
			ytInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			ytInterface->getScrubBarProgress()->setColor(normalColor);
			ytInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}

		if (ytInterface->getVolumeControl()) {
			auto volumeControl = ytInterface->getVolumeControl();
			volumeControl->setMuteImage("%APP%/data/images/waffles/icons/4x/Mute_256.png");
			volumeControl->setVolumeLowImage("%APP%/data/images/waffles/icons/4x/Volume low_256.png");
			volumeControl->setVolumeHighImage("%APP%/data/images/waffles/icons/4x/Volume high_256.png");
			volumeControl->setSliderHeight(8.f);
			volumeControl->setNubSize(12.f);

			volumeControl->setStyle(ds::ui::VideoVolumeStyle::SLIDER);
			auto sliderSprites = volumeControl->getSliderSprites();
			ytInterface->setButtonColor(sliderSprites.mMuteButton, normalColor, highColor);
			ytInterface->setButtonColor(sliderSprites.mVolHighButton, normalColor, highColor);
			ytInterface->setButtonColor(sliderSprites.mVolLowButton, normalColor, highColor);

			sliderSprites.mSliderTrack->setColor(normalColor);
			sliderSprites.mSliderTrack->setOpacity(0.2);
			sliderSprites.mSliderTrack->setCornerRadius(cornerRad);

			sliderSprites.mSliderFill->setColor(normalColor);
			sliderSprites.mSliderFill->setCornerRadius(cornerRad);

			sliderSprites.mSliderNub->setColor(normalColor);
			sliderSprites.mSliderNub->setScale(sliderSprites.mSliderNub->getScale() * 1.25f);
			sliderSprites.mSliderNub->setCornerRadius(100.f);

			/* volumeControl->setStyle(ds::ui::VideoVolumeStyle::CLASSIC);
			auto volumeBars = ytInterface->getVolumeControl()->getBars();
			for (auto eachBar : volumeBars) {
				eachBar->setColor(normalColor);
				eachBar->setCornerRadius(cornerRad);
			}
			*/
		}
	}

	auto webInterface = dynamic_cast<ds::ui::WebInterface*>(interfacey);
	if (webInterface) {
		webInterface->setKeyboardDisablesTimeout(false);
		/* TODO: getKeyboardArea() doesn't exist
		if (auto keebArea = webInterface->getKeyboardArea()) {
			keebArea->setCornerRadius(0.f);
		}
		*/
		if (auto uppy = webInterface->getKeyboardButton()) {
			webInterface->setButtonColor(uppy->getCheckedButton(), highColor, normalColor); // checked
			webInterface->setButtonColor(uppy->getUncheckedButton(), normalColor, highColor); // unchecked
		}
		if (auto downy = webInterface->getBackButton()) {
			webInterface->setButtonColor(downy, normalColor, highColor); 
		}
		if (auto forward = webInterface->getForwardButton()) {
			webInterface->setButtonColor(forward, normalColor, highColor); // forward button
		}
		if (auto refresh = webInterface->getRefreshButton()) {
			webInterface->setButtonColor(refresh, normalColor, highColor); // refresh button
		}
		if (auto lock = webInterface->getTouchToggleButton()) {
			webInterface->setButtonColor(lock->getUncheckedButton(), normalColor, highColor); 
			webInterface->setButtonColor(lock->getCheckedButton(), highColor,
										 normalColor);
		}
	}

	auto pdfInterface = dynamic_cast<ds::ui::PDFInterface*>(interfacey);
	if (pdfInterface) {
		if (auto uppy = pdfInterface->getUpButton()) {
			pdfInterface->setButtonColor(uppy, normalColor, highColor);
		}
		if (auto downy = pdfInterface->getDownButton()) {
			pdfInterface->setButtonColor(downy, normalColor, highColor);
		}
		if (auto toggy = pdfInterface->getTouchToggle()) {
			pdfInterface->setToggleUnlockedColor(normalColor);
			pdfInterface->setToggleLockedColor(highColor);
		}
		if (auto thumbs = pdfInterface->getThumbsButton()) {
			pdfInterface->setButtonColor(thumbs, normalColor, highColor);
		}
		if (auto count = pdfInterface->getPageCounter()) {
			count->setColor(normalColor);
		}
		if (pdfInterface->getScrubBarBackground() && pdfInterface->getScrubBarProgress()) {
			pdfInterface->getScrubBarBackground()->setColor(highColor);
			pdfInterface->getScrubBarBackground()->setCornerRadius(cornerRad);
			pdfInterface->getScrubBarProgress()->setColor(normalColor);
			pdfInterface->getScrubBarProgress()->setCornerRadius(cornerRad);
		}
	}

	interfacey->setBackgroundColor(viewerBackground);
	interfacey->getBackground()->setCornerRadius(cornerRad);
	interfacey->setScale(interfaceScale, interfaceScale);
	if (mEngine.getAppSettings().getString("app:mode", 0, "single") == "multi") {
		interfacey->move(mEngine.getWafflesSettings().getFloat("media_viewer:multi_offset", 0, 0.f), 0.f);
	}
}

// contentHelper functions
ds::Resource BaseWafflesHelper::getBackgroundForPlatform() {
	return {ds::Environment::expand("%APP%/data/images/waffles/default_background.jpg")};
}

int BaseWafflesHelper::getBackgroundPdfPage() {
	return 0;
}

std::vector<ContentModelRef> BaseWafflesHelper::getContentForPlatform() {
	std::vector<ContentModelRef> theList;

	if (mEventFieldKey.empty() || mPlatformFieldKey.empty()) {
		loadIntegration();
	}

	// check if events have playlists
	auto allPlatformEvents = mEngine.mContent.getChildByName("current_content.current_events").getChildren();
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {

			if (!event.getPropertyString(mEventFieldKey).empty()) {

				auto contentUids = ci::split(event.getPropertyString(mEventFieldKey), ",");
				for (auto& uid : contentUids) {
					auto content = getRecordByUid(uid);
					if (!content.empty()) {
						theList.push_back(content);
					}
				}
			}
		}
	} else {
		// DS_LOG_VERBOSE(1, "No scheduled ambient for platform" << myPlatform.getPropertyString("name"))
	}

	auto platform			= getPlatformModel();
	auto defaultContentUids = ci::split(platform.getPropertyString(mPlatformFieldKey), ",");
	for (auto& uid : defaultContentUids) {
		auto content = getRecordByUid(uid);
		if (!content.empty()) {
			theList.push_back(content);
		}
	}

	if (theList.empty() && mUseRoot) {
		return BaseContentHelper::getContentForPlatform();
	}

	return theList;
}

void BaseWafflesHelper::loadIntegration() const {
	mEventFieldKey	  = mEngine.getWafflesSettings().getString("waffles:content:event_field", 0, "additional_content");
	mPlatformFieldKey = mEngine.getWafflesSettings().getString("waffles:content:platform_field", 0, "default_content");
	mUseRoot		  = mEngine.getWafflesSettings().getBool("waffles:use_root_as_fallback", 0, false);


	auto annotationCnt = mEngine.getWafflesSettings().countSetting("annotation:folder:key");
	for (int i = 0; i < annotationCnt; ++i) {
		auto annotationKey = mEngine.getWafflesSettings().getString("annotation:folder:key", i, "");
		mAnnotationFolderKeys.push_back(annotationKey);
	}
	// annotation_folder is always valid?
	mAnnotationFolderKeys.emplace_back("annotation_folder");
}

void BaseWafflesHelper::setLauncherCustomFilters(CustomFilters cf) {
	mLauncherCustomFilters = std::move(cf);
}

const BaseWafflesHelper::CustomFilters& BaseWafflesHelper::getLauncherCustomFilters() {
	return mLauncherCustomFilters;
}

void BaseWafflesHelper::setLauncherCustomContent(CustomContent cc) {
	mLauncherCustomContent = std::move(cc);
}

const BaseWafflesHelper::CustomContent& BaseWafflesHelper::getLauncherCustomContent() {
	return mLauncherCustomContent;
}

bool BaseWafflesHelper::isValidForFilter(const std::string& filter, ContentModelRef model) {
	auto propertyKey = getMediaPropertyKey(model);
	if (mLauncherCustomFilters.find(filter) != mLauncherCustomFilters.end()) {
		return mLauncherCustomFilters[filter](model);
	} else if (filter == "images") {
		return isValidMedia(model, WAFFLESCATEGORY) && model.getPropertyResource(propertyKey).getType() == ds::Resource::IMAGE_TYPE;
	} else if (filter == "presentations") {
		return isValidPlaylist(model, PRESENTATIONCATEGORY); // TODO: untested
	} else if (filter == "videos") {
		return isValidMedia(model, WAFFLESCATEGORY) && (model.getPropertyResource(propertyKey).getType() == ds::Resource::VIDEO_TYPE ||
														model.getPropertyResource(propertyKey).getType() == ds::Resource::YOUTUBE_TYPE);
	} else if (filter == "streams") {
		return isValidMedia(model, WAFFLESCATEGORY) && model.getPropertyResource(propertyKey).getType() == ds::Resource::VIDEO_STREAM_TYPE;
	} else if (filter == "pdfs") {
		return isValidMedia(model, WAFFLESCATEGORY) && model.getPropertyResource(propertyKey).getType() == ds::Resource::PDF_TYPE;
	} else if (filter == "links") {
		return isValidMedia(model, WAFFLESCATEGORY) && model.getPropertyResource(propertyKey).getType() == ds::Resource::WEB_TYPE;
	} else if (filter == "folders") {
		return isValidFolder(model, WAFFLESCATEGORY);
	} else if (filter == "content") {
		return isValidMedia(model, WAFFLESCATEGORY) || isValidFolder(model, WAFFLESCATEGORY) || isValidPlaylist(model, PRESENTATIONCATEGORY);
	}
	return false;
}

} // namespace waffles
