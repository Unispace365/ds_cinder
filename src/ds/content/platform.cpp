#include "platform.h"
#include "stdafx.h"
#include <ds/content/platform.h>
#include <ds/content/content_events.h>

namespace ds::model {

const PlatformType		   Platform::UNDEFINED = "undefined";
ds::model::ContentModelRef Platform::getRecordByUid(const ds::model::ContentModelRef& model,
														 const std::string&				   uid) {
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

	mCurrentContent = mEngine.mContent.getChildByName("current_content");
	if (mCurrentContent.empty()) {
		mCurrentContent.setName("current_content");
		mEngine.mContent.replaceChild(mCurrentContent);
	}

	auto key = platformKey;
	if (key.empty()) {
		key = engine.getAppSettings().getString("platform:key", 0, "");
	}
	auto recordsSize = engine.mContent.getKeyReferences(ds::model::RECORD_MAP).size();
	mEventClient.listenToEvents<ds::ContentUpdatedEvent>([this](const ds::ContentUpdatedEvent& e) {
		refreshContent();
	});
	mPlatformModel = mEngine.mContent.getKeyReference(ds::model::RECORD_MAP, key);
	
	if (mPlatformModel.empty()) {
		DS_LOG_WARNING("Platform not found: " << key << " in " << recordsSize << " records");
		mInitialized = false;
	}

	mEvents = mCurrentContent.getChildByName("current_events");

	mPlatformKey = key;

}

Platform::~Platform() {}

std::string Platform::getPlatformKey() {
	return mPlatformKey;
}

void Platform::refreshContent() {
	mPlatformModel = mEngine.mContent.getKeyReference(ds::model::RECORD_MAP, mPlatformKey);
}

ds::model::ContentModelRef Platform::getPlatformModel() {
	return mPlatformModel;
}

PlatformType Platform::getPlatformType() {
	return mPlatformType;
}

ds::model::ContentModelRef Platform::getCurrentContent() {
	return mCurrentContent;
}

} // namespace ds::model