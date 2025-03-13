#include "stdafx.h"

#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_events.h"
#include "ds/app/engine/engine_service.h"
#include "ds/debug/computer_info.h"
#include "ds/ui/soft_keyboard/entry_field.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"

#include <utility>

namespace ds::ui {

SpriteEngine::SpriteEngine(EngineData& ed, int appMode)
  : mData(ed)
  , mRegisteredEntryField(nullptr)
  , mAppMode(appMode)
  , mRestartAfterUpdate(false)
  , mCallbackId(0) {
	mComputerInfo = new ComputerInfo();
}

SpriteEngine::~SpriteEngine() {
	mData.clearServices();
}

EventNotifier& SpriteEngine::getNotifier() const {
	return mData.mNotifier;
}

void SpriteEngine::loadSettings(const std::string& name, const std::string& filename) {
	mData.mEngineCfg.loadSettings(name, filename);
}

EngineCfg& SpriteEngine::getEngineCfg() {
	return mData.mEngineCfg;
}

const EngineCfg& SpriteEngine::getEngineCfg() const {
	return mData.mEngineCfg;
}

const TextStyle& SpriteEngine::getTextStyle(const std::string& textName) const {
	return mData.mEngineCfg.getTextStyle(textName);
}

cfg::Settings& SpriteEngine::getSettings(const std::string& name) const {
	return mData.mEngineCfg.getSettings(name);
}

cfg::Settings& SpriteEngine::getEngineSettings() const {
	return mData.mEngineCfg.getSettings("engine");
}

cfg::Settings& SpriteEngine::getAppSettings() const {
	return mData.mEngineCfg.getSettings("app_settings");
}

cfg::Settings& SpriteEngine::getWafflesSettings() const {
	return mData.mEngineCfg.getSettings("waffles");
}

cfg::Settings& SpriteEngine::getColorSettings() const {
	return mData.mEngineCfg.getSettings("styles");
}

cfg::Settings& SpriteEngine::getWflColorSettings() const {
	return mData.mEngineCfg.getSettings("waffles_styles");
}

float SpriteEngine::getMinTouchDistance() const {
	return mData.mMinTouchDistance;
}

float SpriteEngine::getMinTapDistance() const {
	return mData.mMinTapDistance;
}

unsigned SpriteEngine::getSwipeQueueSize() const {
	return mData.mSwipeQueueSize;
}

float SpriteEngine::getSwipeMinVelocity() const {
	return mData.mSwipeMinVelocity;
}

float SpriteEngine::getSwipeMaxTime() const {
	return mData.mSwipeMaxTime;
}

float SpriteEngine::getDoubleTapTime() const {
	return mData.mDoubleTapTime;
}

const ci::Rectf& SpriteEngine::getSrcRect() const {
	return mData.mSrcRect;
}

const ci::Rectf& SpriteEngine::getDstRect() const {
	return mData.mDstRect;
}

float SpriteEngine::getWidth() const {
	return mData.mDstRect.getWidth();
}

float SpriteEngine::getHeight() const {
	return mData.mDstRect.getHeight();
}

float SpriteEngine::getWorldWidth() const {
	return mData.mWorldSize.x;
}

float SpriteEngine::getWorldHeight() const {
	return mData.mWorldSize.y;
}

void SpriteEngine::addToDragDestinationList(Sprite* sprite) {
	if (!sprite) return;

	removeFromDragDestinationList(sprite);

	mDragDestinationSprites.push_back(sprite);
}

void SpriteEngine::removeFromDragDestinationList(const Sprite* sprite) {
	if (!sprite) return;

	auto found = std::find(mDragDestinationSprites.begin(), mDragDestinationSprites.end(), sprite);
	if (found != mDragDestinationSprites.end()) mDragDestinationSprites.erase(found);
}

Sprite* SpriteEngine::getDragDestinationSprite(const ci::vec3& globalPoint, const Sprite* draggingSprite) const {
	for (auto sprite : mDragDestinationSprites) {
		if (sprite == draggingSprite) continue;
		if (sprite->contains(globalPoint)) return sprite;
	}

	return nullptr;
}

float SpriteEngine::getFrameRate() const {
	return mData.mFrameRate;
}

void SpriteEngine::setLayoutTarget(std::string target, int index) const {
	auto& setting	  = getEngineSettings().getSetting("xml_importer:target", index);
	setting.mRawValue = std::move(target);
}

bool SpriteEngine::hasLayoutTarget(const std::string &target) const {
	if (target.empty()) return false;
	std::regex regex{"(\\s*,\\s*)"};

	auto end		= std::sregex_token_iterator();
	auto target_itr = std::sregex_token_iterator(target.begin(), target.end(), regex, -1);

	while (target_itr != end) {
		auto target_count = getEngineSettings().countSetting("xml_importer:target");
		for (int i = 0; i < target_count; i++) {

			auto set_target = getEngineSettings().getString("xml_importer:target", i);
			auto set_itr	= std::sregex_token_iterator(set_target.begin(), set_target.end(), regex, -1);
			while (set_itr != end) {
				auto set_value	  = set_itr->str();
				auto target_value = target_itr->str();
				if (set_value == target_value) {
					return true;
				}
				++set_itr;
			}
		}
		++target_itr;
	}
	return false;
}

std::string SpriteEngine::getLayoutTarget(int index) const {
	return getEngineSettings().getString("xml_importer:target", index);
}


const std::string& SpriteEngine::getCmsURL() const {
	return mData.mCmsURL;
}

double SpriteEngine::getElapsedTimeSeconds() {
	return ci::app::getElapsedSeconds();
}

int SpriteEngine::getIdleTimeout() const {
	return mData.mIdleTimeout;
}

void SpriteEngine::setIdleTimeout(int idleTimeout) const {
	mData.mIdleTimeout = idleTimeout;
}

void SpriteEngine::clearFingers(const std::vector<int>& fingers) {}


ComputerInfo& SpriteEngine::getComputerInfo() const {
	assert(mComputerInfo);
	return *mComputerInfo;
}


bool SpriteEngine::getMute() const {
	return mData.mMute;
}


void SpriteEngine::setMute(bool mute) const {
	mData.mMute = mute;
}

const std::string& SpriteEngine::getAppInstanceName() const {
	return mData.mAppInstanceName;
}

bool SpriteEngine::hasService(const std::string& key) const {
	return mData.mServices.find(key) != mData.mServices.cend();
}

EngineService& SpriteEngine::privateGetService(const std::string& str) const {
	EngineService* s = mData.mServices[str];
	if (!s) {
		const std::string msg = "Service (" + str + ") does not exist";
		DS_LOG_FATAL(msg);
	}
	return *s;
}

void SpriteEngine::registerSpriteImporter(const std::string& spriteType, std::function<Sprite*(SpriteEngine&)> func) {
	auto finder = mImporterMap.find(spriteType);
	if (finder != mImporterMap.end()) {
		DS_LOG_WARNING("Duplicate sprite importer being added for sprite type: " << spriteType);
	}

	mImporterMap[spriteType] = func;
}

Sprite* SpriteEngine::createSpriteImporter(const std::string& spriteType) {
	auto finder = mImporterMap.find(spriteType);
	if (finder == mImporterMap.end()) {
		// Not really an error, since the sprite could be created in another manner
		// DS_LOG_WARNING("No importer found for sprite type " << spriteType);
		return nullptr;
	}

	return finder->second(*this);
}

void SpriteEngine::registerSpritePropertySetter(
	const std::string&																					 propertyName,
	std::function<void(Sprite& theSprite, const std::string& theValue, const std::string& fileRefferer)> func) {
	auto finder = mPropertyMap.find(propertyName);
	if (finder != mPropertyMap.end()) {
		DS_LOG_WARNING("Duplicate sprite property setters registered for property name: " << propertyName);
	}

	mPropertyMap[propertyName] = std::move(func);
}


bool SpriteEngine::setRegisteredSpriteProperty(const std::string& propertyName, Sprite& theSprite,
											   const std::string& theValue, const std::string& fileRefferer) {
	auto finder = mPropertyMap.find(propertyName);
	if (finder == mPropertyMap.end()) {
		return false;
	}

	finder->second(theSprite, theValue, fileRefferer);
	return true;
}

void SpriteEngine::registerEntryField(IEntryField* entryField) {
	mRegisteredEntryField = entryField;
	getNotifier().notify(app::EntryFieldRegisteredEvent());
}

IEntryField* SpriteEngine::getRegisteredEntryField() const {
	return mRegisteredEntryField;
}


size_t SpriteEngine::timedCallback(const std::function<void()>& func, double timerSeconds) {
	auto theCallback = new time::Callback(*this);
	if (!theCallback) {
		DS_LOG_WARNING("Couldn't create a timed callback! That's a big deal!");
		return 0;
	}
	mTimedCallbacks.emplace_back(theCallback);
	auto wrappedCallback = [this, func, theCallback] {
		func();
		for (auto it = mTimedCallbacks.begin(); it < mTimedCallbacks.end(); ++it) {
			if ((*it) == theCallback) {
				mTimedCallbacks.erase(it);
				break;
			}
		}
	};

	theCallback->timedCallback(wrappedCallback, timerSeconds);

	return theCallback->getId();
}

size_t SpriteEngine::repeatedCallback(std::function<void()> func, double timerSeconds) {
	auto theCallback = new time::Callback(*this);
	if (!theCallback) {
		DS_LOG_WARNING("Couldn't create a repeated callback! That's a big deal!");
		return 0;
	}
	mTimedCallbacks.emplace_back(theCallback);
	return theCallback->repeatedCallback(std::move(func), timerSeconds);
}

void SpriteEngine::cancelTimedCallback(size_t callbackId) {
	for (auto it = mTimedCallbacks.begin(); it < mTimedCallbacks.end(); ++it) {
		if ((*it)->getId() == callbackId) {
			(*it)->cancel();
			mTimedCallbacks.erase(it);
			break;
		}
	}
}

void SpriteEngine::restartAfterNextUpdate() {
	mRestartAfterUpdate = true;
}

bool SpriteEngine::getRestartAfterNextUpdate() {
	bool doRestart		= mRestartAfterUpdate;
	mRestartAfterUpdate = false;
	return doRestart;
}

std::function<void(VideoPlayer*)> SpriteEngine::getGlobalVideoPlayerCreatedCallback() {
	return mGlobalVideoPlayerCreatedCallback;
}

void SpriteEngine::setGlobalVideoPlayerCreatedCallback(std::function<void(VideoPlayer*)> func) {
	mGlobalVideoPlayerCreatedCallback = std::move(func);
}

float SpriteEngine::getAnimDur() const {
	return mData.mAnimDur;
}

void SpriteEngine::setAnimDur(float newAnimDur) const {
	mData.mAnimDur = newAnimDur;
}

} // namespace ds::ui
