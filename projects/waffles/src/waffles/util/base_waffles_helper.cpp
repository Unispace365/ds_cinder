#include "stdafx.h"
#include "base_waffles_helper.h"
#include "ds/content/platform.h"

namespace waffles {
BaseWafflesHelper::BaseWafflesHelper(ds::ui::SpriteEngine& eng):WafflesHelper(eng),mBaseContentHelper(eng) {}
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
		if (record.getPropertyString("type_key") == "annotation_folder") {
			if (!record.empty()) {
				return record;
			}
		}
	}

	return return_folder;
}

//contentHelper functions
std::string BaseWafflesHelper::getCompositeKeyForPlatform() {
	return mBaseContentHelper.getCompositeKeyForPlatform();
}
ds::model::ContentModelRef BaseWafflesHelper::getRecordByUid(std::string uid) {
	return mBaseContentHelper.getRecordByUid(uid);
}
ds::Resource BaseWafflesHelper::getBackgroundForPlatform() {
	return mBaseContentHelper.getBackgroundForPlatform();
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
	return mBaseContentHelper.getContentForPlatform();
}
std::vector<ds::Resource> BaseWafflesHelper::findMediaResources() {
	return mBaseContentHelper.findMediaResources();
}
} // namespace waffles