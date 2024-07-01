#include "stdafx.h"
#include <ds/content/platform.h>
#include <ds/content/content_events.h>

namespace ds::model {

ds::model::ContentModelRef Platform::getRecordByUid(const ds::model::ContentModelRef& model, const std::string& uid) {
	const auto& children = model.getChildren();
	const auto findy = std::find_if(children.begin(), children.end(),
		[&uid](auto model) { return model.getPropertyString("uid") == uid; });
	if (findy != children.end()) {
		return *findy;
	}

	return {};
}

ds::model::ContentModelRef Platform::getRecordByUid(const ds::ui::SpriteEngine& engine, const std::string& uid) {
	return getRecordByUid(engine.mContent.getChildByName("all_records"), uid);
}

Platform::Platform(ds::ui::SpriteEngine& engine, const std::string& platformKey)
  : mEngine(engine) 
  , mEventClient(engine) {

	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) {
		refreshContent();
	});
	mPlatform = getRecordByUid(engine, platformKey);
	if (mPlatform.empty()) {
		DS_LOG_WARNING("Platform not found: " << platformKey);
	}

	mPlatformKey = platformKey;
}

Platform::~Platform() {}

void Platform::setCurrentPlaylist(std::string playlistKey) {
	mCurrentPlaylist = getRecordByUid(mEngine, playlistKey);
}

ds::model::ContentModelRef Platform::getCurrentPlaylist() {
	return mCurrentPlaylist;
}

std::string Platform::getPlatformKey() {
	return mPlatformKey;
}

void Platform::refreshContent() {
	mPlatform = getRecordByUid(mEngine, mPlatformKey);

	// Consider making the playlist type key a member variable? Not every platform has a default playlist?
	auto defaultPlaylist = mPlatform.getPropertyString("default_playlist");
	if (!defaultPlaylist.empty()) {
		mCurrentPlaylist	 = getRecordByUid(mEngine, defaultPlaylist);
	}
}

ds::model::ContentModelRef Platform::getPlatform()
{
	return mPlatform;
}

ds::model::ContentModelRef Platform::getPlatformProperty(const std::string& key) {
	return mPlatform.getPropertyString(key);
}

} // namespace ds::model