#include "stdafx.h"
#include "base_content_helper.h"
#include "ds/content/platform.h"

namespace ds::model {
BaseContentHelper::BaseContentHelper(ds::ui::SpriteEngine& eng):ContentHelper(eng) {}
BaseContentHelper::~BaseContentHelper() {}

std::string BaseContentHelper::getCompositeKeyForPlatform() {
	//TODO: get key value pairs from waffles_app.xml
	auto key=mEngine.getWafflesSettings().getString("composite:key",0,"");
	return key;
}
ds::model::ContentModelRef BaseContentHelper::getRecordByUid(std::string uid) {
	return mEngine.mContent.getKeyReference(ds::model::VALID_MAP, uid);
}

ds::Resource BaseContentHelper::getBackgroundForPlatform() {
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


ds::model::ContentModelRef BaseContentHelper::getPresentation() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "presentation";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return ds::model::ContentModelRef();
	}
	return playlists.front();
}

ds::model::ContentModelRef BaseContentHelper::getAmbientPlaylist() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "ambient_playlist";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return ds::model::ContentModelRef();
	}
	return playlists.front();
}
std::string BaseContentHelper::getInitialPresentationUid() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "presentation";
	auto playlists = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return "";
	}
	return playlists.front().getUid();
}

std::vector<ds::model::ContentModelRef> BaseContentHelper::getContentForPlatform() {
	
	auto allValid = mEngine.mContent.getChildByName(ds::model::CONTENT).getChildren();
	auto allContent = std::vector<ds::model::ContentModelRef>();

	for (auto value : allValid) {
		allContent.push_back(value);
	}
	return allContent;
}

std::vector<ds::Resource> BaseContentHelper::findMediaResources() {
	return std::vector<ds::Resource>();
}
std::vector<ds::model::ContentModelRef> BaseContentHelper::getFilteredPlaylists(const PlaylistFilter& filter) {

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

} // namespace waffles