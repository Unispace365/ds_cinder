#include "stdafx.h"
#include "base_waffles_helper.h"
#include "ds/content/platform.h"
#include "ds/ui/media/media_interface.h"
#include <ds/ui/media/interface/pdf_interface.h>
#include <ds/ui/media/interface/video_interface.h>
#include <ds/ui/media/interface/video_volume_control.h>
#include <ds/ui/media/interface/web_interface.h>
#include <ds/ui/media/interface/youtube_interface.h>
#include <ds/ui/button/image_button.h>


namespace waffles {
	BaseWafflesHelper::BaseWafflesHelper(ds::ui::SpriteEngine& eng) :WafflesHelper(eng), mBaseContentHelper(eng) { 
		loadIntegration(); 
	
		auto foldersCount = mEngine.getAppSettings().countSetting("content:folder:key");

		for (int i = 0; i < foldersCount; ++i) {
			auto folder = mEngine.getAppSettings().getString("content:folder:key", i, "");
			auto category = mEngine.getAppSettings().getAttribute("content:folder:key", 0, "category", DEFAULTCATEGORY);
			mAcceptableFolders[category].push_back(folder);
			if (mAcceptableFolders[DEFAULTCATEGORY].empty()) mAcceptableFolders[DEFAULTCATEGORY].push_back(folder);
		}

		auto mediaCount = mEngine.getAppSettings().countSetting("content:media:key");
		for (int i = 0; i < mediaCount; ++i) {
			auto media = mEngine.getAppSettings().getString("content:media:key", i);
			auto mediaProp = mEngine.getAppSettings().getAttribute("content:media:key", i, "property_key", "");
			auto category = mEngine.getAppSettings().getAttribute("content:media:key", i, "category", DEFAULTCATEGORY);
			mMediaProps[category][media] = mediaProp;
			if (mMediaProps[DEFAULTCATEGORY][media].empty()) mMediaProps[DEFAULTCATEGORY][media] = mediaProp;
			mAcceptableMedia[category].push_back(media);
			if (mAcceptableMedia[DEFAULTCATEGORY].empty()) mAcceptableMedia[DEFAULTCATEGORY].push_back(media);
		}

		auto playlistCount = mEngine.getAppSettings().countSetting("content:playlist:key");
		for (int i = 0; i < playlistCount; ++i) {
			auto playlist = mEngine.getAppSettings().getString("content:playlist:key", i, "");
			auto category = mEngine.getAppSettings().getAttribute("content:playlist:key", i, "category", DEFAULTCATEGORY);
			mAcceptablePlaylists[category].push_back(playlist);
			if (mAcceptablePlaylists[DEFAULTCATEGORY].empty()) mAcceptablePlaylists[DEFAULTCATEGORY].push_back(playlist);
		}
	}
BaseWafflesHelper::~BaseWafflesHelper() {}


bool BaseWafflesHelper::getApplyParticles() {
	ds::model::Platform platformObj(mEngine);
	auto				platform = platformObj.getPlatformModel();
	if (platform.empty()) return ds::model::ContentModelRef();

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "scheduled_content_event" &&
				!event.getProperty("particle_effect").empty()) {

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


ds::model::ContentModelRef BaseWafflesHelper::getPinboard() {
	auto pinboards = getValidPinboards();
	if (pinboards.empty()) {
		return ds::model::ContentModelRef();
	}
	return pinboards.front();
}


std::vector<ds::model::ContentModelRef> BaseWafflesHelper::getValidPinboards() {
	ds::model::Platform platformObj(mEngine);
	auto				platform = platformObj.getPlatformModel();
	if (platform.empty()) return std::vector<ds::model::ContentModelRef>();

	std::vector<ds::model::ContentModelRef> pinboards;

	// get all the events scheduled for this platform
	auto allPlatformEvents = platform.getChildByName("current_events").getChildren();
	for (const auto& event : allPlatformEvents) {
		if (event.getPropertyString("type_key") == "pinboard_event") {
			pinboards.push_back(event);
		}
	}
	if (pinboards.empty()) {
		pinboards.push_back(ds::model::ContentModelRef());
		//DS_LOG_VERBOSE(1, "No scheduled pinboard for platform" << myPlatform.getPropertyString("name"))
	}
	return pinboards;
}


ds::model::ContentModelRef BaseWafflesHelper::getAnnotationFolder() {
	ds::model::ContentModelRef return_folder;
	int						   count = 0;
	for (const auto& record : mEngine.mContent.getChildByName(ds::model::ALL_RECORDS).getChildren()) {
		auto type = record.getPropertyString("type_key");
		auto valid = std::find(mAnnotationFolderKeys.begin(), mAnnotationFolderKeys.end(), type)!=mAnnotationFolderKeys.end();
		if (valid) {
			if (!record.empty()) {
				return record;
			}
		}
	}

	return return_folder;
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

	const auto imageFlags = ds::ui::Image::IMG_ENABLE_MIPMAP_F | ds::ui::Image::IMG_CACHE_F;

	if (auto vidInterface = dynamic_cast<ds::ui::VideoInterface*>(interfacey)) {
		auto interfaceHeight = vidInterface->getHeight();
		if (auto play = vidInterface->getPlayButton()) {
			play->setNormalImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			play->setHighImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			play->setScale(interfaceHeight / play->getHeight());
			play->setNormalImageColor(normalColor);
			play->setHighImageColor(highColor);
		}

		if (auto pause = vidInterface->getPauseButton()) {
			pause->setNormalImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			pause->setHighImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			pause->setScale(interfaceHeight / pause->getHeight());
			pause->setNormalImageColor(normalColor);
			pause->setHighImageColor(highColor);
		}

		if (auto loopy = vidInterface->getLoopButton()) {
			loopy->setNormalImage("%APP%/data/images/waffles/icons/4x/Loop_256.png", imageFlags);
			loopy->setHighImage("%APP%/data/images/waffles/icons/4x/No loop_256.png", imageFlags);
			loopy->setScale(interfaceHeight / loopy->getHeight());
			loopy->setNormalImageColor(normalColor);
			loopy->setHighImageColor(highColor);
		}

		if (auto unloopy = vidInterface->getUnLoopButton()) {
			unloopy->setNormalImage("%APP%/data/images/waffles/icons/4x/No loop_256.png", imageFlags);
			unloopy->setHighImage("%APP%/data/images/waffles/icons/4x/Loop_256.png", imageFlags);
			unloopy->setScale(interfaceHeight / unloopy->getHeight());
			unloopy->setNormalImageColor(normalColor);
			unloopy->setHighImageColor(highColor);
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
			sliderSprites.mMuteButton->setNormalImageColor(normalColor);
			sliderSprites.mMuteButton->setHighImageColor(highColor);
			sliderSprites.mMuteButton->setScale(sliderSprites.mMuteButton->getScale() * 1.25f);

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
			play->setNormalImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			play->setHighImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			play->setScale(interfaceHeight / play->getHeight());
			play->setNormalImageColor(normalColor);
			play->setHighImageColor(highColor);
		}

		if (auto pause = ytInterface->getPauseButton()) {
			pause->setNormalImage("%APP%/data/images/waffles/icons/4x/Pause_256.png", imageFlags);
			pause->setHighImage("%APP%/data/images/waffles/icons/4x/Play_256.png", imageFlags);
			pause->setScale(interfaceHeight / pause->getHeight());
			pause->setNormalImageColor(normalColor);
			pause->setHighImageColor(highColor);
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
			sliderSprites.mMuteButton->setNormalImageColor(normalColor);
			sliderSprites.mMuteButton->setHighImageColor(highColor);
			sliderSprites.mMuteButton->setScale(sliderSprites.mMuteButton->getScale() * 1.25f);

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
			uppy->setNormalImageColor(normalColor);
			uppy->setHighImageColor(highColor);
			uppy->setCornerRadius(0.f);
		}
		if (auto downy = webInterface->getBackButton()) {
			downy->setNormalImageColor(normalColor);
			downy->setHighImageColor(highColor);
			downy->setCornerRadius(0.f);
		}
		if (auto toggy = webInterface->getForwardButton()) {
			toggy->setNormalImageColor(normalColor);
			toggy->setHighImageColor(highColor);
			toggy->setCornerRadius(0.f);
		}
		if (auto thumbs = webInterface->getRefreshButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
			thumbs->setCornerRadius(0.f);
		}
		if (auto thumbs = webInterface->getTouchToggleButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
			thumbs->setCornerRadius(0.f);
		}
	}

	auto pdfInterface = dynamic_cast<ds::ui::PDFInterface*>(interfacey);
	if (pdfInterface) {
		if (auto uppy = pdfInterface->getUpButton()) {
			uppy->setNormalImageColor(normalColor);
			uppy->setHighImageColor(highColor);
		}
		if (auto downy = pdfInterface->getDownButton()) {
			downy->setNormalImageColor(normalColor);
			downy->setHighImageColor(highColor);
		}
		if (auto toggy = pdfInterface->getTouchToggle()) {
			toggy->setNormalImageColor(normalColor);
			toggy->setHighImageColor(highColor);
		}
		if (auto thumbs = pdfInterface->getThumbsButton()) {
			thumbs->setNormalImageColor(normalColor);
			thumbs->setHighImageColor(highColor);
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

//contentHelper functions
std::string BaseWafflesHelper::getCompositeKeyForPlatform() {
	return mBaseContentHelper.getCompositeKeyForPlatform();
}
ds::model::ContentModelRef BaseWafflesHelper::getRecordByUid(std::string uid) {
	return mBaseContentHelper.getRecordByUid(uid);
}
ds::Resource BaseWafflesHelper::getBackgroundForPlatform() {
	//return mBaseContentHelper.getBackgroundForPlatform();

	return ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/default_background.jpg"));
}
int BaseWafflesHelper::getBackgroundPdfPage() {
	return 0;
}
ds::model::ContentModelRef BaseWafflesHelper::getPresentation() {
	return mBaseContentHelper.getPresentation();
}
ds::model::ContentModelRef BaseWafflesHelper::getAmbientPlaylist() {
	return mBaseContentHelper.getAmbientPlaylist();
}
std::string BaseWafflesHelper::getInitialPresentationUid() {
	return mBaseContentHelper.getInitialPresentationUid();
}
std::vector<ds::model::ContentModelRef> BaseWafflesHelper::getFilteredPlaylists(const PlaylistFilter& filter) {
	return mBaseContentHelper.getFilteredPlaylists(filter);
}
std::vector<ds::model::ContentModelRef> BaseWafflesHelper::getContentForPlatform() {
	ds::model::Platform platformObj(mEngine);
	auto				platformCurrent = platformObj.getCurrentContent();
	auto 			    platform = platformObj.getPlatformModel();
	//if (platformCurrent.empty()) return std::vector<ds::model::ContentModelRef>();

	// get all the events scheduled for this platform
	auto allPlatformEvents = platformCurrent.getChildByName("current_events").getChildren();

	std::vector<ds::model::ContentModelRef> theList;
	
	// check if events have playlists
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
	}
	else {
		//DS_LOG_VERBOSE(1, "No scheduled ambient for platform" << myPlatform.getPropertyString("name"))
	}


	auto defaultContentUids = ci::split(platform.getPropertyString(mPlatformFieldKey), ",");
	for (auto& uid : defaultContentUids) {
		auto content = getRecordByUid(uid);
		if (!content.empty()) {
			theList.push_back(content);
		}
	}

	if (theList.empty() && mUseRoot) {
		return mBaseContentHelper.getContentForPlatform();
	}

	return theList;
}
std::vector<ds::Resource> BaseWafflesHelper::findMediaResources() {
	return mBaseContentHelper.findMediaResources();
}
void BaseWafflesHelper::loadIntegration()
{

	mEventFieldKey = mEngine.getWafflesSettings().getString("waffles:content:event_field", 0, "additional_content");
	mPlatformFieldKey = mEngine.getWafflesSettings().getString("waffles:content:platform_field", 0, "default_content");
	mUseRoot = mEngine.getWafflesSettings().getBool("waffles:use_root_as_fallback", 0, false);


	auto annotationCnt = mEngine.getWafflesSettings().countSetting("annotation:folder:key");
	for (int i = 0; i < annotationCnt; ++i) {
		auto annotationKey = mEngine.getWafflesSettings().getString("annotation:folder:key", i, "");
		mAnnotationFolderKeys.push_back(annotationKey);
	}
	//annotation_folder is always valid?
	mAnnotationFolderKeys.push_back("annotation_folder");


}

bool BaseWafflesHelper::isValidFolder(ds::model::ContentModelRef model, std::string category) {
	return mBaseContentHelper.isValidFolder(model, category);
}

bool BaseWafflesHelper::isValidMedia(ds::model::ContentModelRef model, std::string category) {
	return mBaseContentHelper.isValidMedia(model, category);
}

bool BaseWafflesHelper::isValidPlaylist(ds::model::ContentModelRef model, std::string category) {
	return mBaseContentHelper.isValidPlaylist(model, category);
}

std::string BaseWafflesHelper::getMediaPropertyKey(ds::model::ContentModelRef model, std::string category) {
	return mBaseContentHelper.getMediaPropertyKey(model, category);
}

void BaseWafflesHelper::setLauncherCustomFilters(std::unordered_map<std::string, std::function<bool(ds::model::ContentModelRef)>> cf) {
	mLauncherCustomFilters = cf;
}

std::unordered_map<std::string, std::function<bool(ds::model::ContentModelRef)>> BaseWafflesHelper::getLauncherCustomFilters() {
	return mLauncherCustomFilters;
}

bool BaseWafflesHelper::isValidForFilter(std::string filter, ds::model::ContentModelRef model) {
	auto property_key = getMediaPropertyKey(model);
	if (filter == "images") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) && 
			   model.getPropertyResource(property_key).getType() == ds::Resource::IMAGE_TYPE;
	} else if (filter == "presentations") {
		return isValidPlaylist(model, ds::model::ContentHelper::PRESENTATIONCATEGORY); // TODO: untested
	} else if (filter == "videos") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) &&
			   (model.getPropertyResource(property_key).getType() == ds::Resource::VIDEO_TYPE ||
				model.getPropertyResource(property_key).getType() == ds::Resource::YOUTUBE_TYPE);
	} else if (filter == "streams") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) &&
			   model.getPropertyResource(property_key).getType() == ds::Resource::VIDEO_STREAM_TYPE;
	} else if (filter == "pdfs") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) &&
			   model.getPropertyResource(property_key).getType() == ds::Resource::PDF_TYPE;
	} else if (filter == "links") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) &&
			   model.getPropertyResource(property_key).getType() == ds::Resource::WEB_TYPE;
	} else if (filter == "folders") {
		return isValidFolder(model, ds::model::ContentHelper::WAFFLESCATEGORY);
	} else if (filter == "content") {
		return isValidMedia(model, ds::model::ContentHelper::WAFFLESCATEGORY) ||
			   isValidFolder(model, ds::model::ContentHelper::WAFFLESCATEGORY) ||
			   isValidPlaylist(model, ds::model::ContentHelper::PRESENTATIONCATEGORY);
	} else { // TODO: have everything above map based like the customs
		if (mLauncherCustomFilters.find(filter) != mLauncherCustomFilters.end()) {
			return mLauncherCustomFilters[filter](model);
		}
	}
	return false;
}

} // namespace waffles