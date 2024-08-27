#include "stdafx.h"
#include "base_waffles_helper.h"
#include "ds/content/platform.h"

namespace waffles {
	BaseWafflesHelper::BaseWafflesHelper(ds::ui::SpriteEngine& eng) :WafflesHelper(eng), mBaseContentHelper(eng) { loadIntegration(); }
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
} // namespace waffles