#include "stdafx.h"

#include "app/cms_definitions.hpp"
#include "app/helpers.h"

#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/sprite/gst_video.h>
#include <ds/util/string_util.h>

namespace downstream {

Platform getPlatformType(const ds::ui::SpriteEngine& engine) {
	return UNDEFINED;
}

std::string getPlatformKey(const ds::ui::SpriteEngine& engine) {
	auto platformKey = engine.getAppSettings().getString("platform:key", 0, "");
	if (!platformKey.empty()) {
		return platformKey;
	}
	auto platformName = engine.getAppSettings().getString("app:platform", 0, "");
	if (platformName == "some_platform") {
		return waffles::cms::platform::somePlatform;
	}
	return "";
}

bool isPlatform(const ds::model::ContentModelRef& model) {
	const auto uid = model.getPropertyString("type_uid");
	return uid == waffles::cms::type::somePlatformType;
}

std::string getPlatformPlaylistId(const ds::model::ContentModelRef& platform) {
	const std::string platformName = platform.getPropertyValue("type_key");
	for (auto event : platform.getChildByName("current_events").getChildren()) {
		auto eventKey = event.getPropertyString("type_key");
		if (platformName == "some_platform") {
			if(eventKey == "some_event"){
				return event.getPropertyString("some_playlist");
			}
		} else {
			DS_LOG_ERROR("no playlist associated");
		}
	}

	// Fallback to platform playlist
	if (platformName == "some_platform") {
		return platform.getPropertyString("some_playlist");
	} else {
		DS_LOG_ERROR("no playlist associated");
	}

	return "";
}

ds::model::ContentModelRef getPlatformPlaylist(const ds::ui::SpriteEngine& engine) {
	const auto platform	  = getPlatform(engine);
	const auto playlistId = getPlatformPlaylistId(platform);
	return getRecordByUid(engine, playlistId);
}

ds::model::ContentModelRef getRecordByUid(const ds::ui::SpriteEngine& engine, const std::string& uid) {
	return getRecordByUid(engine.mContent.getChildByName("all_records"), uid);
}

ds::model::ContentModelRef getPlatform(const ds::ui::SpriteEngine& engine) {
	return getRecordByUid(engine, getPlatformKey(engine));
}

ds::model::ContentModelRef getPinboard(const ds::ui::SpriteEngine& engine) {
	return getPinboard(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

ds::model::ContentModelRef getAnnotationFolder(const ds::ui::SpriteEngine& engine) {
	return getAnnotationFolder(engine.mContent.getChildByName("all_records"));
}

std::vector<ds::model::ContentModelRef> getStreams(const ds::ui::SpriteEngine& engine) {
	return getStreams(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

ds::model::ContentModelRef getInteractivePlaylist(const ds::ui::SpriteEngine& engine) {
	return getInteractivePlaylist(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

ds::model::ContentModelRef getAmbientPlaylist(const ds::ui::SpriteEngine& engine) {
	return getAmbientPlaylist(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

ds::Resource getBackground(const ds::ui::SpriteEngine& engine) {
	return getBackground(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

std::string getInitialPresentation(const ds::ui::SpriteEngine& engine) {
	return getInitialPresentation(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

bool getApplyParticles(const ds::ui::SpriteEngine& engine) {
	return getApplyParticles(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

std::vector<ds::model::ContentModelRef> getAssets(ds::ui::SpriteEngine& engine) {
	return getAssets(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

std::vector<ds::model::ContentModelRef> getAllValid(ds::ui::SpriteEngine& engine) {
	return getAllValid(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

std::vector<ds::model::ContentModelRef> getValidPinboards(const ds::ui::SpriteEngine& engine) {
	return getValidPinboards(engine.mContent.getChildByName("all_records"), getPlatformKey(engine));
}

ds::model::ContentModelRef getRecordByUid(const ds::model::ContentModelRef& records, const std::string& uid) {
	const auto& allRecords = records.getChildren();

	const auto findy = std::find_if(allRecords.begin(), allRecords.end(),
									[&uid](auto model) { return model.getPropertyString("uid") == uid; });
	if (findy != allRecords.end()) {
		return *findy;
	}

	return ds::model::ContentModelRef();
}

ds::model::ContentModelRef getPlatform(const ds::model::ContentModelRef& records, const std::string& platformKey) {
	return getRecordByUid(records, platformKey);
}

ds::model::ContentModelRef getPinboard(const ds::model::ContentModelRef& records, const std::string& platformKey) {
	auto thePlatform = getPlatform(records, platformKey);
	if (thePlatform.empty()) return ds::model::ContentModelRef();

	auto currentEvents = thePlatform.getChildByName("current_events");
	for (const auto& ev : currentEvents.getChildren()) {
		if (ev.getPropertyString("type_key") == "pinboard_event") {
			return ev;
		}
	}

	return ds::model::ContentModelRef();
}

ds::model::ContentModelRef getAnnotationFolder(const ds::model::ContentModelRef& records) {

	ds::model::ContentModelRef return_folder;
	int						   count = 0;
	for (const auto& record : records.getChildren()) {
		if (record.getPropertyString("type_key") == "annotation_folder") {
			if (return_folder.empty()) {
				return_folder = record;
				count++;
			}
		}
	}

	return return_folder;
}

std::vector<ds::model::ContentModelRef> getStreams(const ds::model::ContentModelRef& records,
												   const std::string&				 platformKey) {
	auto thePlatform = getPlatform(records, platformKey);
	if (thePlatform.empty()) return std::vector<ds::model::ContentModelRef>();

	std::vector<ds::model::ContentModelRef> theStreams;
	for (const auto& ch : thePlatform.getChildren()) {
		if (ch.getPropertyString("type_key") == "stream") {
			theStreams.push_back(ch);
		}
	}

	return theStreams;
}

ds::model::ContentModelRef getPlaylist(const ds::model::ContentModelRef& records, const std::string& platformKey,
									   const std::string& eventTypeKey) {
	const auto platform = getPlatform(records, platformKey);
	if (platform.empty()) return ds::model::ContentModelRef();

	ds::model::ContentModelRef thePlaylist;

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == eventTypeKey &&
				!event.getPropertyString("ambient_playlist").empty()) {

				const auto playlistSelection = ci::split(event.getPropertyString("ambient_playlist"), ",");
				thePlaylist					 = getRecordByUid(records, playlistSelection.front());
				break;
			}
		}
	} else {
		// DS_LOG_VERBOSE(1, "No scheduled ambient playlist for platform" << platform.getPropertyString("name"))
	}

	// if there is no event playlist scheduled then check default platform playlists
	const auto platformDefaultAmbientPlaylistId =
		ci::split(platform.getPropertyString("default_ambient_playlist"), ",");
	if (thePlaylist.empty() && !platformDefaultAmbientPlaylistId.empty()) {
		thePlaylist = getRecordByUid(records, platformDefaultAmbientPlaylistId.front());
	}

	return thePlaylist;
}

ds::model::ContentModelRef getInteractivePlaylist(const ds::model::ContentModelRef& records,
												  const std::string&				platformKey) {
	return getPlaylist(records, platformKey, "some_playlist");
}

ds::model::ContentModelRef getAmbientPlaylist(const ds::model::ContentModelRef& records,
											  const std::string&				platformKey) {
	return getPlaylist(records, platformKey, "some_playlist");
}

ds::Resource getBackground(const ds::model::ContentModelRef& records, const std::string& platformKey) {
	auto platform = getPlatform(records, platformKey);
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

std::string getInitialPresentation(const ds::model::ContentModelRef& records, const std::string& platformKey) {
	auto platform = getPlatform(records, platformKey);
	if (platform.empty()) return "";

	// get all the events scheduled for this platform, already sorted in order of importance
	const auto& allPlatformEvents = platform.getChildByName("current_events").getChildren();

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "some_event" &&
				!event.getPropertyString("initial-presentation").empty()) {

				return event.getPropertyString("initial-presentation");
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled initial presentation for platform" << platform.getPropertyString("name"))
	}

	if (!platform.getPropertyString("initial-presentation").empty()) {
		return platform.getPropertyString("initial-presentation");
	}

	DS_LOG_VERBOSE(1, "No initial presentation for platform '" << platform.getPropertyString("name") << "'.");

	return "";
}

std::string getPlatformCompositeFrameKey(const ds::ui::SpriteEngine& engine) {
	auto platformType = getPlatformType(engine);
	if (platformType == UNDEFINED) {
		return "";
	}
}

bool getApplyParticles(const ds::model::ContentModelRef& records, const std::string& platformKey) {
	auto platform = getPlatform(records, platformKey);
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

std::vector<ds::model::ContentModelRef> getAssets(const ds::model::ContentModelRef& records,
												  const std::string&				platformKey) {
	ds::model::ContentModelRef myPlatform = getPlatform(records, platformKey);
	if (myPlatform.empty()) return std::vector<ds::model::ContentModelRef>();

	// get all the events scheduled for this platform
	auto allPlatformEvents = myPlatform.getChildByName("current_events").getChildren();

	std::vector<ds::model::ContentModelRef> theList;
	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "some_event" &&
				!event.getPropertyString("additional_content").empty()) {

				auto contentUids = ci::split(event.getPropertyString("additional_content"), ",");
				for (auto& uid : contentUids) {
					auto content = getRecordByUid(records, uid);
					theList.push_back(content);
				}
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled ambient for platform" << myPlatform.getPropertyString("name"))
	}

	// if there is no event playlist scheduled then check default platform playlists
	auto defaultContentUids = ci::split(myPlatform.getPropertyString("default_content"), ",");
	for (auto& uid : defaultContentUids) {
		auto content = getRecordByUid(records, uid);
		theList.push_back(content);
	}

	return theList;
}

std::vector<ds::model::ContentModelRef> getAllValid(const ds::model::ContentModelRef& records,
													const std::string&				  platformKey) {
	ds::model::ContentModelRef myPlatform = getPlatform(records, platformKey);
	if (myPlatform.empty()) return std::vector<ds::model::ContentModelRef>();

	// get all the events scheduled for this platform
	auto allPlatformEvents = myPlatform.getChildByName("current_events").getChildren();

	std::vector<ds::model::ContentModelRef> theList;

	// assets
	auto defaultContentUids = ci::split(myPlatform.getPropertyString("default_content"), ",");
	for (auto& uid : defaultContentUids) {
		auto content = getRecordByUid(records, uid);
		theList.push_back(content);
	}

	// get all platform streams
	auto allStreams = getStreams(records, platformKey);
	auto streams	= ds::model::ContentModelRef("Streams");
	streams.setProperty("type_key", std::string("streams"));
	streams.setProperty("record_name", std::string("Streams"));
	streams.setChildren(allStreams);
	if (!allStreams.empty()) theList.push_back(streams);

	auto playlists = ds::model::ContentModelRef("Playlists");
	playlists.setProperty("type_key", std::string("playlist_folder"));
	playlists.setProperty("record_name", std::string("Playlists"));
	std::vector<ds::model::ContentModelRef> playlistChildren;

	// check if events have playlists
	if (!allPlatformEvents.empty()) {
		for (const auto& event : allPlatformEvents) {
			if (event.getPropertyString("type_key") == "some_event") {

				for (auto& uid : ci::split(event.getPropertyString("additional_content"), ",")) {
					auto repeat = false;
					for (auto default_uid : defaultContentUids) {
						if (uid == default_uid) {
							repeat = true;
							break;
						}
					}
					if (!repeat) {
						auto content = getRecordByUid(records, uid);
						theList.push_back(content);
					}
				}

				if (!event.getPropertyString("ambient_playlist").empty()) {
					for (const auto& item : ci::split(event.getPropertyString("ambient_playlist"), ",")) {
						playlistChildren.push_back(getRecordByUid(records, item));
					}
				}
			}
		}
	} else {
		DS_LOG_VERBOSE(1, "No scheduled ambient for platform" << myPlatform.getPropertyString("name"))
	}

	auto platformDefaultAmbientPlaylistId = ci::split(myPlatform.getPropertyString("default_ambient_playlist"), ",");

	if (!platformDefaultAmbientPlaylistId.empty()) {
		auto findy = std::find_if(playlistChildren.begin(), playlistChildren.end(),
								  [id = platformDefaultAmbientPlaylistId.front()](auto& item) {
									  return item.getPropertyString("uid") == id;
								  });
		if (findy == playlistChildren.end()) {
			playlistChildren.push_back(getRecordByUid(records, platformDefaultAmbientPlaylistId.front()));
		}
	}

	playlists.setChildren(playlistChildren);

	// theList.push_back(playlists); // TODO: do we want playlist functionality?

	return theList;
}

std::vector<ds::model::ContentModelRef> getValidPinboards(const ds::model::ContentModelRef& records,
														  const std::string&				platformKey) {
	ds::model::ContentModelRef myPlatform = getPlatform(records, platformKey);
	if (myPlatform.empty()) return std::vector<ds::model::ContentModelRef>();

	std::vector<ds::model::ContentModelRef> pinboards;

	// get all the events scheduled for this platform
	auto allPlatformEvents = myPlatform.getChildByName("current_events").getChildren();
	for (const auto& event : allPlatformEvents) {
		if (event.getPropertyString("type_key") == "pinboard_event") {
			pinboards.push_back(event);
		}
	}
	return pinboards;
}

ds::ui::MediaPlayer* findMediaPlayer(ds::ui::Sprite* sprite) {
	if (!sprite) return nullptr;

	auto media = dynamic_cast<ds::ui::MediaPlayer*>(sprite);
	if (media /*&& !isBackgroundMedia(media)*/) return media;

	auto children = sprite->getChildren();
	for (auto child : children) {
		media = findMediaPlayer(child);
		if (media) return media;
	}

	return nullptr;
}

std::vector<ds::ui::MediaPlayer*> findMediaPlayers(ds::ui::Sprite* sprite) {
	std::vector<ds::ui::MediaPlayer*> players;

	if (sprite) {
		auto media = dynamic_cast<ds::ui::MediaPlayer*>(sprite);
		if (media /*&& !isBackgroundMedia(media)*/) players.push_back(media);

		auto children = sprite->getChildren();
		for (auto child : children) {
			auto childPlayers = findMediaPlayers(child);
			players.insert(players.end(), childPlayers.begin(), childPlayers.end());
		}
	}

	return players;
}

bool rewindMedia(ds::ui::MediaPlayer* player) {
	if (player) {
		auto video = dynamic_cast<ds::ui::VideoPlayer*>(player->getPlayer());
		if (video) {
			auto stream = video->getVideo();
			if (stream) {
				DS_LOG_VERBOSE(1, "Rewinding media: " << player->getResource().getFileName())
				stream->playAFrame(0, nullptr, true);
				return true;
			}
			DS_LOG_VERBOSE(1, "Failed to rewind media: " << player->getResource().getFileName())
		}
	}
	return false;
}

bool pauseMedia(ds::ui::MediaPlayer* player) {
	if (player) {
		auto video = dynamic_cast<ds::ui::VideoPlayer*>(player->getPlayer());
		if (video) {
			auto stream = video->getVideo();
			if (stream) {
				DS_LOG_VERBOSE(1, "Pausing media: " << player->getResource().getFileName())
				stream->pause();
				return true;
			}
			DS_LOG_VERBOSE(1, "Failed to pause media: " << player->getResource().getFileName())
		}
	}
	return false;
}

bool playMedia(ds::ui::MediaPlayer* player) {
	if (player) {
		auto video = dynamic_cast<ds::ui::VideoPlayer*>(player->getPlayer());
		if (video) {
			auto stream = video->getVideo();
			if (stream) {
				DS_LOG_VERBOSE(1, "Playing media: " << player->getResource().getFileName())
				stream->play();
				return true;
			}
			DS_LOG_VERBOSE(1, "Failed to play media: " << player->getResource().getFileName())
		}
	}
	return false;
}

bool isBackgroundMedia(const ds::ui::MediaPlayer* player) {
	if (!player) return false;

	const auto name = ds::utf8_from_wstr(player->getSpriteName());
	return name.find("background") != std::string::npos;
}

std::vector<ds::Resource> findMediaResources(const ds::model::ContentModelRef& model) {
	std::vector<ds::Resource> resources;

	const auto& props = model.getProperties();
	for (auto& [name, content] : props) {
		auto resource = content.getResource();
		if (!resource.empty()) {
			// Skip background and preview resources.
			if (name.find("background") != std::string::npos || name.find("preview") != std::string::npos) continue;

			if (resource.getType() == ds::Resource::WEB_TYPE)
				resources.push_back(resource);
			else if (resource.getType() == ds::Resource::VIDEO_TYPE)
				resources.push_back(resource);
		}
	}

	const auto& children = model.getChildren();
	for (const auto& child : children) {
		const auto nested = findMediaResources(child);
		resources.insert(resources.end(), nested.begin(), nested.end());
	}

	return resources;
}

bool findBackgroundColor(ds::ui::SpriteEngine& engine, const ds::model::ContentModelRef& model, ci::Color& color) {
	if (model.getProperty("background_color").empty()) {
		for (const auto& child : model.getChildren()) {
			if (findBackgroundColor(engine, child, color)) return true;
		}
		return false;
	}

	color = model.getPropertyColor(engine, "background_color");

	return true;
}

const std::vector<ci::Color8u>& getColors() {
	using namespace waffles::cms::colors;
	static std::vector<ci::Color8u> colors = {bloom, midnight, bloom020, noon};
	return colors;
}

const std::vector<ci::Color8u>& getAccentColors() {
	using namespace waffles::cms::colors;
	static std::vector<ci::Color8u> colors = {accent::agave, accent::gold,	  accent::lavender,
											  accent::rose,	 accent::saffron, accent::spearmint};
	return colors;
}

const std::vector<ci::Color8u>& getHighlightColors() {
	using namespace waffles::cms::colors;
	static std::vector<ci::Color8u> colors = {
		highlight::bloom, highlight::midnight, highlight::bloom020, highlight::noon,	highlight::agave,
		highlight::gold,  highlight::lavender, highlight::rose,		highlight::saffron, highlight::spearmint};
	return colors;
}

const std::vector<ci::Color8u>& getShadowColors() {
	using namespace waffles::cms::colors;
	static std::vector<ci::Color8u> colors = {shadow::bloom,   shadow::midnight, shadow::bloom020, shadow::noon,
											  shadow::agave,   shadow::gold,	 shadow::lavender, shadow::rose,
											  shadow::saffron, shadow::spearmint};
	return colors;
}

} // namespace downstream
