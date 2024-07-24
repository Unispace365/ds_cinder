#include "stdafx.h"
#include "base_waffles_helper.h"
#include "ds/content/platform.h"

namespace waffles {
BaseWafflesHelper::BaseWafflesHelper(ds::ui::SpriteEngine& eng):WafflesHelper(eng) {}
BaseWafflesHelper::~BaseWafflesHelper() {}

std::string BaseWafflesHelper::getCompositeKeyForPlatform() {
	//TODO: get key value pairs from waffles_app.xml
	return std::string();
}
ds::model::ContentModelRef BaseWafflesHelper::getRecordByUid(std::string uid) {
	return mEngine.mContent.getKeyReference(ds::model::VALID_MAP, uid);
}

ds::Resource BaseWafflesHelper::getBackgroundForPlatform() {
	ds::model::Platform platformObj(mEngine); 
	auto platform = platformObj.getPlatformModel();
	if (platform.empty()) return ds::Resource();

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "some_event" &&
				!event.getPropertyResource("content-browsing-background").empty()) {

				return event.getPropertyResource("content-browsing-background");
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled background for platform" << platform.getPropertyString("name"))
	}

	if (!platform.getPropertyResource("content-browsing-background").empty()) {
		return platform.getPropertyResource("content-browsing-background");
	}

	DS_LOG_VERBOSE(1, "No platform background for platform '" << platform.getPropertyString("name")
															  << "'. Using default background.");

	return ds::Resource(ds::Environment::expand("%APP%/data/images/default_background.png"));
}

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
ds::model::ContentModelRef BaseWafflesHelper::getPresentation() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "presentation";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return ds::model::ContentModelRef();
	}
	return playlists.front();
}

ds::model::ContentModelRef BaseWafflesHelper::getAmbientPlaylist() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "ambient_playlist";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return ds::model::ContentModelRef();
	}
	return playlists.front();
}
std::string BaseWafflesHelper::getInitialPresentationUid() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "presentation";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return "";
	}
	return playlists.front().getUid();
}

ds::model::ContentModelRef BaseWafflesHelper::getPinboard() {
	auto pinboards = getValidPinboards();
	if (pinboards.empty()) {
		return ds::model::ContentModelRef();
	}
	return pinboards.front();
}
std::vector<ds::model::ContentModelRef> BaseWafflesHelper::getContentForPlatform() {
	ds::model::Platform platformObj(mEngine);
	auto				platform = platformObj.getPlatformModel();
	if (platform.empty()) return std::vector<ds::model::ContentModelRef>();

	// get all the events scheduled for this platform
	auto allPlatformEvents = platform.getChildByName("current_events").getChildren();

	std::vector<ds::model::ContentModelRef> theList;
	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (!event.getPropertyString("additional_content").empty()) {

				auto contentUids = ci::split(event.getPropertyString("additional_content"), ",");
				for (auto& uid : contentUids) {
					auto content = getRecordByUid(uid);
					theList.push_back(content);
				}
			}
		}
	} else {
		//DS_LOG_VERBOSE(1, "No scheduled ambient for platform" << myPlatform.getPropertyString("name"))
	}

	
	auto defaultContentUids = ci::split(platform.getPropertyString("default_content"), ",");
	for (auto& uid : defaultContentUids) {
		auto content = getRecordByUid(uid);
		theList.push_back(content);
	}

	return theList;
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
std::vector<ds::Resource> BaseWafflesHelper::findMediaResources() {
	return std::vector<ds::Resource>();
}
std::vector<ds::model::ContentModelRef> BaseWafflesHelper::getFilteredPlaylists(const PlaylistFilter& filter) {

	auto				eventPropName = filter.eventPropertyName.empty() ? "playlist" : filter.eventPropertyName;
	auto				platformPropName = filter.platformPropertyName.empty() ? "default_playlist" : filter.platformPropertyName;
	ds::model::Platform platformObj(mEngine);
	auto				platform = platformObj.getPlatformModel();
	if (platform.empty()) return std::vector<ds::model::ContentModelRef>();

	ds::model::ContentModelRef thePlaylist;

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();
	std::vector<ds::model::ContentModelRef> thePlaylists;
	
	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			auto eventTypeKey = event.getPropertyString("type_key");
			if (
				(eventTypeKey == filter.eventTypeKey || filter.eventTypeKey.empty()) &&
				!event.getPropertyString(eventPropName).empty()) {

				const auto playlistSelection = ci::split(event.getPropertyString(eventPropName), ",");
				//check each playlist for correct type_key
				for (const auto& playlistUid : playlistSelection) {
					auto playlist = getRecordByUid(playlistUid);
					if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
						thePlaylists.push_back(playlist);
					}
				}
			}
		}
	} else {
		// DS_LOG_VERBOSE(1, "No scheduled ambient playlist for platform" << platform.getPropertyString("name"))
	}

	// if there is no event playlist scheduled then check default platform playlists
	const auto platformDefaultAmbientPlaylistId =
		ci::split(platform.getPropertyString(platformPropName), ",");
	if (!platformDefaultAmbientPlaylistId.empty()) {
		//add to the playlist if filterMode is All, or clear the playlist if filterMode is PlatformOverride and if the filterMode is PlatformFallback and the playlist is empty skip this step
		if (filter.filterMode == PlaylistFilter::FilterMode::All) {
			for (const auto& playlistUid : platformDefaultAmbientPlaylistId) {
				auto playlist = getRecordByUid(playlistUid);
				if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
					thePlaylists.push_back(playlist);
				}
			}
		}
		else if (filter.filterMode == PlaylistFilter::FilterMode::PlatformOverride) {
			thePlaylists.clear();
			for (const auto& playlistUid : platformDefaultAmbientPlaylistId) {
				auto playlist = getRecordByUid(playlistUid);
				if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
					thePlaylists.push_back(playlist);
				}
			}
		}
		else if (filter.filterMode == PlaylistFilter::FilterMode::PlatformFallback && thePlaylists.empty()) {
			for (const auto& playlistUid : platformDefaultAmbientPlaylistId) {
				auto playlist = getRecordByUid(playlistUid);
				if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
					thePlaylists.push_back(playlist);
				}
			}
		}
		
	}

	return thePlaylists;
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
} // namespace waffles