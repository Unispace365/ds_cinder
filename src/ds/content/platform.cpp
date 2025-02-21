#include "stdafx.h"

#include <ds/content/content_events.h>
#include <ds/content/platform.h>

namespace ds::model {

const PlatformType Platform::UNDEFINED = "undefined";

ContentModelRef Platform::getRecordByUid(const ContentModelRef& model, const std::string& uid) {
	const auto& children = model.getChildren();
	const auto	findy	 = std::find_if(children.begin(), children.end(),
										[&uid](auto model) { return model.getPropertyString("uid") == uid; });
	if (findy != children.end()) {
		return *findy;
	}

	return {};
}

ContentModelRef Platform::getRecordByUid(const ui::SpriteEngine& engine, const std::string& uid) {
	return getRecordByUid(engine.mContent.getChildByName("all_records"), uid);
}

Platform::Platform(ui::SpriteEngine& engine, const std::string& platformKey)
  : mEngine(engine)
  , mEventClient(engine) {

	mCurrentContent = mEngine.mContent.getChildByName("current_content");
	if (mCurrentContent.empty()) {
		mCurrentContent.setName("current_content");
		mEngine.mContent.replaceChild(mCurrentContent);
	}

	auto key = platformKey;
	if (key.empty()) {
		key = engine.getAppSettings().getString("platform:key", 0, "");
	}
	auto recordsSize = engine.mContent.getKeyReferences(RECORD_MAP).size();

	mPlatformModel = mEngine.mContent.getKeyReference(RECORD_MAP, key);

	if (mPlatformModel.empty()) {
		DS_LOG_WARNING("Platform not found: " << key << " in " << recordsSize << " records");
		mInitialized = false;
	} else {
		mInitialized = true;
	}

	mEvents = mCurrentContent.getChildByName("current_events");

	mPlatformKey  = key;
	mPlatformType = mPlatformModel.getPropertyString("type_key");
}

const std::string& Platform::getPlatformKey() const {
	return mPlatformKey;
}

void Platform::refreshContent() {
	mPlatformModel = mEngine.mContent.getKeyReference(RECORD_MAP, mPlatformKey);
}

ContentModelRef Platform::getPlatformModel() {
	refreshContent();
	return mPlatformModel;
}

PlatformType Platform::getPlatformType() const {
	return mPlatformType;
}

ContentModelRef Platform::getCurrentContent() const {
	return mCurrentContent;
}

void Platform::setupContentListener() {
	mEventClient.listenToEvents<ContentUpdatedEvent>([this](const ContentUpdatedEvent& e) { refreshContent(); });
}

} // namespace ds::model