#pragma once

#include <cinder/app/App.h>

#include <any>
#include <functional>
#include <ds/app/app.h>
#include <ds/app/event_client.h>

// #include "service/bridge_service.h"

namespace ds { namespace ui {
	class MediaPlayer;
	class SmartLayout;
}} // namespace ds::ui

namespace downstream {
class Platform;
class ContentHelper;
class MediaHelper;
class ColorHelper;
typedef std::shared_ptr<Platform> PlatformPtr;
typedef std::shared_ptr<ContentHelper> ContentHelperPtr;
typedef std::shared_ptr<MediaHelper> MediaHelperPtr;
typedef std::shared_ptr<ColorHelper> ColorHelperPtr;
typedef std::string					   HelperId;
std::string operator""_h(const char* str, size_t len) { return std::string(str, len); }


class HelperFunction {
  public:
	typedef std::function<std::any(void* data)> Function;
	HelperFunction(const Function& func) : mFunction(func) {}
	std::any operator()(void* data) { return mFunction(data); }
	template <typename T>
	HelperFunction& operator=(std::function<T(void*)>& func) {
		this->mFunction = [this](void* data) -> std::any {
			return std::any(func(data));
		};
	}
  private:
	Function mFunction;
};

class Platform {
	Platform(const ds::ui::SpriteEngine& engine,const std::string& key) : mEngine(engine), mKey(key) {}

  private:
	  const ds::ui::SpriteEngine& mEngine;
	  std::string mKey;
};

class Helper {
public:
	Helper(const ds::ui::SpriteEngine& engine) { initializeHelpers(); };
	~Helper() { clearAllHelpers(); };
	virtual void initializeHelpers();
	template <typename T>
	void setHelper(const HelperId& id, const T&& func) {
		mFunctions[id] = new HelperFunction(func);
	};
	template <typename T> T callHelper(const HelperId& id, void* data) {
		auto x = HelperFunction([](void* data) -> int{}); // = mFunctions[id];
		return std::any_cast<T>(x(data));
	};
	
	void clearHelper(const HelperId& id) {
		if (mFunctions.find(id) != mFunctions.end()) {
			delete mFunctions[id];
		}
		mFunctions.erase(id);
	};
	void clearAllHelpers() {
		for (auto it = mFunctions.begin(); it != mFunctions.end(); ++it) {
			delete it->second;
		}
		mFunctions.clear();
	};
  private:
	std::unordered_map<HelperId, HelperFunction*> mFunctions;
};

// Access through sprite engine.
class ContentHelper : public Helper {
  public:
	ContentHelper(const ds::ui::SpriteEngine& engine);
	virtual void initializeHelpers() {
		setHelper(HelperId("content.platform.key"), [this](void* data){ return std::string(""); });
	};
	virtual PlatformPtr	getPlatform();
	virtual std::string getPlatformKey(void* data = nullptr)													= 0;
	virtual bool		isPlatform(const ds::model::ContentModelRef& model, void* data = nullptr)				= 0;
	virtual std::string getPlatformPlaylistId(const ds::model::ContentModelRef& platform, void* data = nullptr) = 0;
	virtual ds::model::ContentModelRef getPlatformPlaylist(void* data = nullptr)								= 0;
	virtual ds::model::ContentModelRef getRecordByUid(const std::string& uid, void* data = nullptr)			= 0;
	virtual ds::model::ContentModelRef getPlatform(void* data = nullptr)										= 0;
	virtual ds::model::ContentModelRef getPinboard(void* data = nullptr)										= 0;
	virtual ds::model::ContentModelRef getAnnotationFolder(void* data = nullptr)								= 0;
	virtual std::vector<ds::model::ContentModelRef> getStreams(void* data = nullptr)							= 0;
	virtual ds::model::ContentModelRef getPlaylist(const std::string& platformKey, const std::string& eventTypeKey, void* data = nullptr) = 0;
	virtual ds::model::ContentModelRef getInteractivePlaylist(void* data = nullptr)							= 0;
	virtual ds::model::ContentModelRef getAmbientPlaylist(void* data = nullptr)								= 0;
	virtual ds::Resource getBackground(void* data = nullptr)													= 0;
	virtual std::string getInitialPresentation(void* data = nullptr)											= 0;
	virtual bool getApplyParticles(void* data = nullptr)														= 0;
	virtual std::vector<ds::model::ContentModelRef> getAssets(void* data = nullptr)							= 0;
	virtual std::vector<ds::model::ContentModelRef> getAllValid(void* data = nullptr)							= 0;
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards(void* data = nullptr)					= 0;


  private:
	ds::ui::SpriteEngine& mEngine;
};


Platform getPlatformType(const ds::ui::SpriteEngine& engine);

std::string getPlatformKey(const ds::ui::SpriteEngine& engine);

bool isPlatform(const ds::model::ContentModelRef& model);

std::string getPlatformPlaylistId(const ds::model::ContentModelRef& platform);

ds::model::ContentModelRef getPlatformPlaylist(const ds::ui::SpriteEngine& engine);

ds::model::ContentModelRef getRecordByUid(const ds::ui::SpriteEngine& engine, const std::string& uid);

ds::model::ContentModelRef getPlatform(const ds::ui::SpriteEngine& engine);

ds::model::ContentModelRef getPinboard(const ds::ui::SpriteEngine& engine);

ds::model::ContentModelRef getAnnotationFolder(const ds::ui::SpriteEngine& engine);

std::vector<ds::model::ContentModelRef> getStreams(const ds::ui::SpriteEngine& engine);

ds::model::ContentModelRef getPlaylist(const ds::model::ContentModelRef& records, const std::string& platformKey,
									   const std::string& eventTypeKey);

ds::model::ContentModelRef getInteractivePlaylist(const ds::ui::SpriteEngine& engine);

ds::model::ContentModelRef getAmbientPlaylist(const ds::ui::SpriteEngine& engine);

ds::Resource getBackground(const ds::ui::SpriteEngine& engine);

std::string getInitialPresentation(const ds::ui::SpriteEngine& engine);

bool getApplyParticles(const ds::ui::SpriteEngine& engine);

std::vector<ds::model::ContentModelRef> getAssets(ds::ui::SpriteEngine& engine);

std::vector<ds::model::ContentModelRef> getAllValid(ds::ui::SpriteEngine& engine);

std::vector<ds::model::ContentModelRef> getValidPinboards(const ds::ui::SpriteEngine& engine);

// Access through content records.

ds::model::ContentModelRef getRecordByUid(const ds::model::ContentModelRef& records, const std::string& uid);

ds::model::ContentModelRef getPlatform(const ds::model::ContentModelRef& records, const std::string& platformKey);

ds::model::ContentModelRef getPinboard(const ds::model::ContentModelRef& records, const std::string& platformKey);

ds::model::ContentModelRef getAnnotationFolder(const ds::model::ContentModelRef& records);

std::vector<ds::model::ContentModelRef> getStreams(const ds::model::ContentModelRef& records,
												   const std::string&				 platformKey);

ds::model::ContentModelRef getInteractivePlaylist(const ds::model::ContentModelRef& records,
												  const std::string&				platformKey);

ds::model::ContentModelRef getAmbientPlaylist(const ds::model::ContentModelRef& records,
											  const std::string&				platformKey);

ds::Resource getBackground(const ds::model::ContentModelRef& records, const std::string& platformKey);

std::string getInitialPresentation(const ds::model::ContentModelRef& records, const std::string& platformKey);

bool getApplyParticles(const ds::model::ContentModelRef& records, const std::string& platformKey);

std::vector<ds::model::ContentModelRef> getAssets(const ds::model::ContentModelRef& records,
												  const std::string&				platformKey);

std::vector<ds::model::ContentModelRef> getAllValid(const ds::model::ContentModelRef& records,
													const std::string&				  platformKey);

std::vector<ds::model::ContentModelRef> getValidPinboards(const ds::model::ContentModelRef& records,
														  const std::string&				platformKey);

std::string getPlatformCompositeFrameKey(const ds::ui::SpriteEngine& engine);

//! Returns the first MediaPlayer found in the hierarchy.
ds::ui::MediaPlayer* findMediaPlayer(ds::ui::Sprite* sprite);
//! Returns all MediaPlayers found in the hierarchy.
std::vector<ds::ui::MediaPlayer*> findMediaPlayers(ds::ui::Sprite* sprite);

bool rewindMedia(ds::ui::MediaPlayer* player);
bool pauseMedia(ds::ui::MediaPlayer* player);
bool playMedia(ds::ui::MediaPlayer* player);

//! Returns whether the specified MediaPlayer is background media.
bool isBackgroundMedia(const ds::ui::MediaPlayer* player);

//! Returns all media (video, web) resources found in the hierarchy.
std::vector<ds::Resource> findMediaResources(const ds::model::ContentModelRef& model);

//!
bool findBackgroundColor(ds::ui::SpriteEngine& engine, const ds::model::ContentModelRef& model, ci::Color& color);

/// Returns the base brand colors.
static const std::vector<ci::Color8u>& getColors();
/// Returns the accent brand colors.
static const std::vector<ci::Color8u>& getAccentColors();
/// Returns the highlight brand colors.
static const std::vector<ci::Color8u>& getHighlightColors();
/// Returns the shadow brand colors.
static const std::vector<ci::Color8u>& getShadowColors();

} // namespace downstream
