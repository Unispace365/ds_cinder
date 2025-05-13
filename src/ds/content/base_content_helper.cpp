#include "stdafx.h"

#include "ds/content/base_content_helper.h"

namespace ds::model {

BaseContentHelper::BaseContentHelper(ui::SpriteEngine& eng)
  : ContentHelper(eng) {}

std::string BaseContentHelper::getPlatformKey() const {
	return mEngine.getAppSettings().getString("platform:key", 0, "");
}

std::string BaseContentHelper::getPlatformType(const std::string& platformKey) const {
	return getPlatformModel(platformKey).getPropertyString("type");
}

const std::vector<ContentModelRef>& BaseContentHelper::getPlatformEvents(const std::string& platformKey) const {
	return getPlatformModel(platformKey).getChildByName("current_events").getChildren();
}

std::string BaseContentHelper::getCompositeKeyForPlatform() {
	// TODO: get key value pairs from waffles_app.xml
	auto key = mEngine.getWafflesSettings().getString("composite:key", 0, "");
	return key;
}

ContentModelRef BaseContentHelper::getRecordByUid(const std::string& uid) const {
	return mEngine.mContent.getKeyReference(VALID_MAP, uid);
}

ContentModelRef BaseContentHelper::getCurrentContent() const {
	// We need to ensure this record is created if it doesn't exist
	auto currentContent = mEngine.mContent.getChildByName(CURRENT_CONTENT);
	if(currentContent.empty()){
		currentContent.setName(CURRENT_CONTENT);
		mEngine.mContent.addChild(currentContent);
	}
	return currentContent;
}

Resource BaseContentHelper::getBackgroundForPlatform() {
	auto platform = getPlatformModel();
	if (platform.empty()) return {};

	// check if events have playlists
	const auto& allPlatformEvents = getPlatformEvents();
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "some_event" && !event.getPropertyResource("content-browsing-background").empty()) {

				return event.getPropertyResource("content-browsing-background");
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled background for platform" << platform.getPropertyString("name"))
	}

	if (!platform.getPropertyResource("content-browsing-background").empty()) {
		return platform.getPropertyResource("content-browsing-background");
	}

	DS_LOG_VERBOSE(1, "No platform background for platform '" << platform.getPropertyString("name") << "'. Using default background.");

	return {Environment::expand("%APP%/data/images/default_background.png")};
}

ContentModelRef BaseContentHelper::getPresentation() {
	PlaylistFilter filter;
	filter.playlistTypeKey		= "presentation";
	filter.platformPropertyName = "default_presentation";
	auto playlists				= getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return {};
	}
	return playlists.front();
}

ContentModelRef BaseContentHelper::getAmbientPlaylist() {
	PlaylistFilter filter;
	filter.playlistTypeKey = "ambient_playlist";
	auto playlists		   = getFilteredPlaylists(filter);
	if (playlists.empty()) {
		return {};
	}
	return playlists.front();
}

std::string BaseContentHelper::getInitialPresentationUid() {
	auto model = getPresentation();
	if (model.empty()) {
		return {};
	}
	return model.getUid();

	// PlaylistFilter filter;
	// filter.playlistTypeKey = "presentation";
	// filter.platformPropertyName = "default_presentation";
	// auto playlists = getFilteredPlaylists(filter);
	// if (playlists.empty()) {
	//	return {};
	// }
	// return playlists.front().getUid();
}

std::vector<ContentModelRef> BaseContentHelper::getContentForPlatform() {
	// Get the source collections.
	auto validContent	 = mEngine.mContent.getChildByName(CONTENT).getChildren();
	auto platformContent = getPlatformModel().getChildren();

	// Reserve space to avoid reallocations.
	std::vector<ContentModelRef> result;
	result.reserve(validContent.size() + platformContent.size());

	// Insert elements efficiently.
	result.insert(result.end(), validContent.begin(), validContent.end());
	result.insert(result.end(), platformContent.begin(), platformContent.end());

	return result;
}

std::vector<Resource> BaseContentHelper::findMediaResources() {
	return {};
}

std::vector<ContentModelRef> BaseContentHelper::getFilteredPlaylists(const PlaylistFilter& filter) {
	auto eventPropName	  = filter.eventPropertyName.empty() ? "playlist" : filter.eventPropertyName;
	auto platformPropName = filter.platformPropertyName.empty() ? "default_playlist" : filter.platformPropertyName;

	auto platform = getPlatformModel();
	if (platform.empty()) return {};

	ContentModelRef				 thePlaylist;
	std::vector<ContentModelRef> thePlaylists;

	// check if events have playlists
	const auto& allPlatformEvents = getPlatformEvents();
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			auto eventTypeKey = event.getPropertyString("type_key");
			if ((eventTypeKey == filter.eventTypeKey || filter.eventTypeKey.empty()) && !event.getPropertyString(eventPropName).empty()) {

				const auto playlistSelection = ci::split(event.getPropertyString(eventPropName), ",");
				// check each playlist for correct type_key
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
	const auto platformDefaultAmbientPlaylistId = ci::split(platform.getPropertyString(platformPropName), ",");
	if (!platformDefaultAmbientPlaylistId.empty()) {
		// add to the playlist if filterMode is All, or clear the playlist if filterMode is PlatformOverride and if the
		// filterMode is PlatformFallback and the playlist is empty skip this step
		if (filter.filterMode == PlaylistFilter::FilterMode::All) {
			for (const auto& playlistUid : platformDefaultAmbientPlaylistId) {
				auto playlist = getRecordByUid(playlistUid);
				if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
					thePlaylists.push_back(playlist);
				}
			}
		} else if (filter.filterMode == PlaylistFilter::FilterMode::PlatformOverride) {
			thePlaylists.clear();
			for (const auto& playlistUid : platformDefaultAmbientPlaylistId) {
				auto playlist = getRecordByUid(playlistUid);
				if (filter.playlistTypeKey.empty() || playlist.getPropertyString("type_key") == filter.playlistTypeKey) {
					thePlaylists.push_back(playlist);
				}
			}
		} else if (filter.filterMode == PlaylistFilter::FilterMode::PlatformFallback && thePlaylists.empty()) {
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

bool BaseContentHelper::isValidFolder(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat = trim(cat);
		auto folders  = getAcceptableFolders(cleanCat);

		if (std::find(folders.begin(), folders.end(), key) != folders.end()) {
			return true;
		}
		if (std::find(folders.begin(), folders.end(), type) != folders.end()) {
			return true;
		}
	}

	return false;
}

bool BaseContentHelper::isValidMedia(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat = trim(cat);
		auto media	  = getAcceptableMedia(cleanCat);

		if (std::find(media.begin(), media.end(), key) != media.end()) {
			return true;
		}
		if (std::find(media.begin(), media.end(), type) != media.end()) {
			return true;
		}
	}

	return false;
}

bool BaseContentHelper::isValidPlaylist(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat  = trim(cat);
		auto playlists = getAcceptablePlaylists(cleanCat);

		if (std::find(playlists.begin(), playlists.end(), key) != playlists.end()) {
			return true;
		}
		if (std::find(playlists.begin(), playlists.end(), type) != playlists.end()) {
			return true;
		}
	}

	return false;
}

std::string BaseContentHelper::getMediaPropertyKey(ContentModelRef model, const std::string& category) {
	auto props		= getMediaProps(category.empty() ? DEFAULTCATEGORY : category); // copy due to the use of operator[]
	auto theType	= model.getPropertyString("type_key");
	auto theTypeUid = model.getPropertyString("type_uid");
	auto media_property_key = props[theTypeUid]; // here
	media_property_key		= media_property_key.empty() ? props[theType] : media_property_key;
	media_property_key		= media_property_key.empty() ? "media" : media_property_key;
	return media_property_key;
}

std::vector<ContentModelRef> BaseContentHelper::getStreamSources(const std::string& category) {
	auto						 platformModel = getPlatformModel();
	auto						 kids		   = platformModel.getChildren();
	std::vector<ContentModelRef> sources;
	for (const auto& model : kids) {
		if (isValidStreamSource(model, category)) {
			sources.push_back(model);
		}
	}
	return sources;
}

ContentModelRef BaseContentHelper::getStreamSourceForStream(ContentModelRef stream, const std::string& category) {
	if (isValidStream(stream, category)) {
		auto streamMatchKey = getStreamMatchKey(stream, category);
		auto sources		= getStreamSources(category);
		for (auto source : sources) {
			auto sourceMatchKey = getStreamMatchKey(source, category);
			auto streamMatch	= stream.getPropertyString(streamMatchKey);
			auto sourceMatch	= source.getPropertyString(sourceMatchKey);
			if (sourceMatch == streamMatch) {
				return source;
			}
		}
	}
	return {};
}

bool BaseContentHelper::isValidStreamSource(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat = trim(cat);
		auto sources  = getAcceptableStreamSources(cleanCat);
		if (std::find(sources.begin(), sources.end(), key) != sources.end()) {
			return true;
		}
		if (std::find(sources.begin(), sources.end(), type) != sources.end()) {
			return true;
		}
	}
	return false;
}

bool BaseContentHelper::isValidStream(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat = trim(cat);
		auto streams  = getAcceptableStreams(cleanCat);
		if (std::find(streams.begin(), streams.end(), key) != streams.end()) {
			return true;
		}
		if (std::find(streams.begin(), streams.end(), type) != streams.end()) {
			return true;
		}
	}
	return false;
}

std::string BaseContentHelper::getStreamMatchKey(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat	 = trim(cat);
		auto streamProps = getStreamMatchProp(cleanCat);
		auto matchKey	 = streamProps[key];
		if (!matchKey.empty()) {
			return matchKey;
		}
		matchKey = streamProps[type];
		if (!matchKey.empty()) {
			return matchKey;
		}
	}
	return {};
}

std::string BaseContentHelper::getStreamSourceAddressKey(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat	 = trim(cat);
		auto streamProps = getStreamSourceAddressProps(cleanCat);
		auto streamKey	 = streamProps[key];
		if (!streamKey.empty()) {
			return streamKey;
		}
		streamKey = streamProps[type];
		if (!streamKey.empty()) {
			return streamKey;
		}
	}
	return {};
}

std::string BaseContentHelper::getStreamSourceTypeKey(ContentModelRef model, const std::string& category) {
	auto categories = split(category.empty() ? DEFAULTCATEGORY : category, ",", true);
	auto type		= model.getPropertyString("type_uid");
	auto key		= model.getPropertyString("type_key");
	// return the type key for the stream source
	for (auto& cat : categories) {
		// trim whitespace from cat using std::find_not_last_of and std::find_not_first_of functions
		auto cleanCat	   = trim(cat);
		auto streamProps   = getStreamSourceTypeProps(cleanCat);
		auto streamTypeKey = streamProps[key];
		if (!streamTypeKey.empty()) {
			return streamTypeKey;
		}
		streamTypeKey = streamProps[type];
		if (!streamTypeKey.empty()) {
			return streamTypeKey;
		}
	}
	return {};
}

const std::vector<std::string>& BaseContentHelper::getAcceptableFolders(const std::string& category) const {
	if (mAcceptableFolders.empty()) {
		auto foldersCount = mEngine.getWafflesSettings().countSetting("content:folder:key");

		for (int i = 0; i < foldersCount; ++i) {
			auto folder = mEngine.getWafflesSettings().getString("content:folder:key", i, "");
			auto cat = mEngine.getWafflesSettings().getAttribute("content:folder:key", 0, "category", DEFAULTCATEGORY);
			mAcceptableFolders[cat].push_back(folder);
			if (mAcceptableFolders[DEFAULTCATEGORY].empty()) mAcceptableFolders[DEFAULTCATEGORY].push_back(folder);
		}
	}

	return mAcceptableFolders[category];
}

const std::vector<std::string>& BaseContentHelper::getAcceptableMedia(const std::string& category) const {
	if (mAcceptableMedia.empty()) {
		initializeAcceptableMedia();
	}

	return mAcceptableMedia[category];
}

const std::vector<std::string>& BaseContentHelper::getAcceptablePlaylists(const std::string& category) const {
	if (mAcceptablePlaylists.empty()) {
		auto playlistCount = mEngine.getWafflesSettings().countSetting("content:playlist:key");
		for (int i = 0; i < playlistCount; ++i) {
			auto playlist = mEngine.getWafflesSettings().getString("content:playlist:key", i, "");
			auto category =
				mEngine.getWafflesSettings().getAttribute("content:playlist:key", i, "category", DEFAULTCATEGORY);
			if (!playlist.empty()) {
				mAcceptablePlaylists[category].push_back(playlist);
				if (mAcceptablePlaylists[DEFAULTCATEGORY].empty())
					mAcceptablePlaylists[DEFAULTCATEGORY].push_back(playlist);
			}
		}
	}

	return mAcceptablePlaylists[category];
}

const std::vector<std::string>& BaseContentHelper::getAcceptableStreamSources(const std::string& category) const {
	if (mAcceptableStreamSources.empty()) {
		initializeStreamSources();
	}
	return mAcceptableStreamSources[category];
}

const std::vector<std::string>& BaseContentHelper::getAcceptableStreams(const std::string& category) const {
	if (mAcceptableStreams.empty()) {
		initializeStreamSources();
	}

	return mAcceptableStreams[category];
}

const std::unordered_map<std::string, std::string>&
BaseContentHelper::getStreamSourceAddressProps(const std::string& category) const {
	if (mStreamSourceAddressProps.empty()) {
		initializeStreamSources();
	}
	return mStreamSourceAddressProps[category];
}

const std::unordered_map<std::string, std::string>&
BaseContentHelper::getStreamSourceTypeProps(const std::string& category) const {
	if (mStreamSourceTypeProps.empty()) {
		initializeStreamSources();
	}
	return mStreamSourceTypeProps[category];
}

const std::unordered_map<std::string, std::string>&
BaseContentHelper::getStreamMatchProp(const std::string& category) const {
	if (mStreamMatchProp.empty()) {
		initializeStreamSources();
	}

	return mStreamMatchProp[category];
}

const std::unordered_map<std::string, std::string>&
BaseContentHelper::getMediaProps(const std::string& category) const {
	if (mMediaProps.empty()) {
		initializeAcceptableMedia();
	}

	return mMediaProps[category];
}

void BaseContentHelper::initializeAcceptableMedia() const {
	auto mediaCount = mEngine.getWafflesSettings().countSetting("content:media:key");
	for (int i = 0; i < mediaCount; ++i) {
		auto media	   = mEngine.getWafflesSettings().getString("content:media:key", i);
		auto mediaProp = mEngine.getWafflesSettings().getAttribute("content:media:key", i, "property_key", "");
		auto category  = mEngine.getWafflesSettings().getAttribute("content:media:key", i, "category", DEFAULTCATEGORY);
		mMediaProps[category][media] = mediaProp;
		if (mMediaProps[DEFAULTCATEGORY][media].empty()) mMediaProps[DEFAULTCATEGORY][media] = mediaProp;
		mAcceptableMedia[category].push_back(media);
		if (mAcceptableMedia[DEFAULTCATEGORY].empty()) mAcceptableMedia[DEFAULTCATEGORY].push_back(media);
	}
}

void BaseContentHelper::initializeStreamSources() const {
	auto streamCount = mEngine.getWafflesSettings().countSetting("content:stream:key");
	for (int i = 0; i < streamCount; ++i) {
		auto stream			 = mEngine.getWafflesSettings().getString("content:stream:key", i, "");
		auto streamMatchProp = mEngine.getWafflesSettings().getAttribute("content:stream:key", i, "match_key", "");

		auto category = mEngine.getWafflesSettings().getAttribute("content:stream:key", i, "category", DEFAULTCATEGORY);
		mStreamMatchProp[category][stream] = streamMatchProp;
		if (mStreamMatchProp[DEFAULTCATEGORY][stream].empty())
			mStreamMatchProp[DEFAULTCATEGORY][stream] = streamMatchProp;
		mAcceptableStreams[category].push_back(stream);
		if (mAcceptableStreams[DEFAULTCATEGORY].empty()) mAcceptableStreams[DEFAULTCATEGORY].push_back(stream);
	}

	auto streamSourceCount = mEngine.getWafflesSettings().countSetting("content:stream_source:key");
	for (int i = 0; i < streamSourceCount; ++i) {
		auto streamSource = mEngine.getWafflesSettings().getString("content:stream_source:key", i, "");
		auto streamSourceAddressProp =
			mEngine.getWafflesSettings().getAttribute("content:stream_source:key", i, "address_key", "");
		auto streamSourceTypeProp =
			mEngine.getWafflesSettings().getAttribute("content:stream_source:key", i, "streamtype_key", "");
		auto streamMatchProp =
			mEngine.getWafflesSettings().getAttribute("content:stream_source:key", i, "match_key", "");
		auto category =
			mEngine.getWafflesSettings().getAttribute("content:stream_source:key", i, "category", DEFAULTCATEGORY);
		mStreamSourceAddressProps[category][streamSource] = streamSourceAddressProp;
		if (mStreamSourceAddressProps[DEFAULTCATEGORY][streamSource].empty())
			mStreamSourceAddressProps[DEFAULTCATEGORY][streamSource] = streamSourceAddressProp;
		mStreamSourceTypeProps[category][streamSource] = streamSourceTypeProp;
		if (mStreamSourceTypeProps[DEFAULTCATEGORY][streamSource].empty())
			mStreamSourceTypeProps[DEFAULTCATEGORY][streamSource] = streamSourceTypeProp;
		mStreamMatchProp[category][streamSource] = streamMatchProp;
		if (mStreamMatchProp[DEFAULTCATEGORY][streamSource].empty())
			mStreamMatchProp[DEFAULTCATEGORY][streamSource] = streamMatchProp;
		mAcceptableStreamSources[category].push_back(streamSource);
		if (mAcceptableStreamSources[DEFAULTCATEGORY].empty())
			mAcceptableStreamSources[DEFAULTCATEGORY].push_back(streamSource);
	}
}

} // namespace ds::model
