#include "stdafx.h"
#include "base_content_helper.h"
#include "ds/content/platform.h"

namespace ds::model {
	BaseContentHelper::BaseContentHelper(ds::ui::SpriteEngine& eng) :ContentHelper(eng) {
		auto foldersCount = mEngine.getWafflesSettings().countSetting("content:folder:key");

		for (int i = 0; i < foldersCount; ++i) {
			auto folder = mEngine.getWafflesSettings().getString("content:folder:key", i, "");
			auto category = mEngine.getWafflesSettings().getAttribute("content:folder:key", 0, "category", DEFAULTCATEGORY);
			mAcceptableFolders[category].push_back(folder);
			if (mAcceptableFolders[DEFAULTCATEGORY].empty()) mAcceptableFolders[DEFAULTCATEGORY].push_back(folder);
		}

		auto mediaCount = mEngine.getWafflesSettings().countSetting("content:media:key");
		for (int i = 0; i < mediaCount; ++i) {
			auto media = mEngine.getWafflesSettings().getString("content:media:key", i);
			auto mediaProp = mEngine.getWafflesSettings().getAttribute("content:media:key", i, "property_key", "");
			auto category = mEngine.getWafflesSettings().getAttribute("content:media:key", i, "category", DEFAULTCATEGORY);
			mMediaProps[category][media] = mediaProp;
			if (mMediaProps[DEFAULTCATEGORY][media].empty()) mMediaProps[DEFAULTCATEGORY][media] = mediaProp;
			mAcceptableMedia[category].push_back(media);
			if (mAcceptableMedia[DEFAULTCATEGORY].empty()) mAcceptableMedia[DEFAULTCATEGORY].push_back(media);
		}

		auto playlistCount = mEngine.getWafflesSettings().countSetting("content:playlist:key");
		for (int i = 0; i < playlistCount; ++i) {
			auto playlist = mEngine.getWafflesSettings().getString("content:playlist:key", i, "");
			auto category = mEngine.getWafflesSettings().getAttribute("content:playlist:key", i, "category", DEFAULTCATEGORY);
			mAcceptablePlaylists[category].push_back(playlist);
			if (mAcceptablePlaylists[DEFAULTCATEGORY].empty()) mAcceptablePlaylists[DEFAULTCATEGORY].push_back(playlist);
		}

	}
	BaseContentHelper::~BaseContentHelper() {}

	std::string BaseContentHelper::getCompositeKeyForPlatform() {
		//TODO: get key value pairs from waffles_app.xml
		auto key = mEngine.getWafflesSettings().getString("composite:key", 0, "");
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
		}
		else {
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
		filter.platformPropertyName = "default_presentation";
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

		auto model = getPresentation();
		if (model.empty()) {
			return "";
		}
		return model.getUid();


		PlaylistFilter filter;
		filter.playlistTypeKey = "presentation";
		filter.platformPropertyName = "default_presentation";
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
		}
		else {
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

	bool BaseContentHelper::isValidFolder(ds::model::ContentModelRef model, std::string category) {
		if (category.empty()) category = DEFAULTCATEGORY;

		auto categories = ds::split(category, ",", true);
		auto type = model.getPropertyString("type_uid");
		auto key = model.getPropertyString("type_key");
		for (auto& cat : categories) {
			//trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions

			auto cleanCat = ds::trim(cat);
			auto folders = mAcceptableFolders[cleanCat];

			if (std::find(folders.begin(), folders.end(), key) != folders.end()) {
				return true;
			}
			if (std::find(folders.begin(), folders.end(), type) != folders.end()) {
				return true;
			}
		}

		return false;
	}

	bool BaseContentHelper::isValidMedia(ds::model::ContentModelRef model, std::string category) {
		if (category.empty()) category = DEFAULTCATEGORY;

		auto categories = ds::split(category, ",", true);
		auto type = model.getPropertyString("type_uid");
		auto key = model.getPropertyString("type_key");
		for (auto& cat : categories) {
			// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions

			auto cleanCat = ds::trim(cat);
			
			if (std::find(mAcceptableMedia[cleanCat].begin(), mAcceptableMedia[cleanCat].end(), key) !=
				mAcceptableMedia[cleanCat].end()) {
				return true;
			}
			if (std::find(mAcceptableMedia[cleanCat].begin(), mAcceptableMedia[cleanCat].end(), type) !=
				mAcceptableMedia[cleanCat].end()) {
				return true;
			}
		}

		return false;

	} // namespace waffles

	bool BaseContentHelper::isValidPlaylist(ds::model::ContentModelRef model, std::string category) {
		if (category.empty()) category = DEFAULTCATEGORY;

		auto categories = ds::split(category, ",", true);
		auto type		= model.getPropertyString("type_uid");
		auto key		= model.getPropertyString("type_key");
		for (auto& cat : categories) {
			// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions

			auto cleanCat  = ds::trim(cat);
			auto playlists = mAcceptablePlaylists[cleanCat];

			if (std::find(playlists.begin(), playlists.end(), key) != playlists.end()) {
				return true;
			}
			if (std::find(playlists.begin(), playlists.end(), type) != playlists.end()) {
				return true;
			}
		}

		return false;
	}

	std::string BaseContentHelper::getMediaPropertyKey(ds::model::ContentModelRef model,std::string category) {
		if (category.empty()) category = DEFAULTCATEGORY;
		auto		theType = model.getPropertyString("type_key");
		auto		theTypeUid = model.getPropertyString("type_uid");
		auto		media_property_key = mMediaProps[category][theTypeUid];
		media_property_key = media_property_key.empty() ? mMediaProps[category][theType] : media_property_key;
		media_property_key = media_property_key.empty() ? "media" : media_property_key;
		return media_property_key;
	}
}