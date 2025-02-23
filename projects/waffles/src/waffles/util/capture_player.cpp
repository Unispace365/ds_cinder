#include "stdafx.h"

#include "capture_player.h"

#include <cinder/Capture.h>

#include <ds/util/string_util.h>

#include <memory>
#include <thread>
#include <mutex>
#include <chrono>

#include <cinder/Thread.h>

namespace Capture {
auto INIT = []() {
	ds::App::AddStartup("CapturePlayer", [](ds::Engine& e) {
		e.registerSpriteImporter("capture_player", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
			return new waffles::CapturePlayer(enginey);
		});

		// TODO: Setting the source via XML property
	});
	return true;
}();

struct cap {
	ci::CaptureRef	   capture;
	int				   users = 0;
};
struct SharedSurface {
    ci::Surface8uRef surface;
    std::mutex mutex;
};
struct SharedTexture {
    ci::gl::Texture2dRef texture;
    std::mutex mutex;
};
// Holds all open captures, allowing us to display multiple copies of a capture while only using one GPU resource
// The first CapturePlayer to update for a given Capture source will update the texture
static std::unordered_map<int64_t, cap>		sCaptures;
static std::unordered_map<std::string, cap> sUNCaptures;
static std::unordered_map<int64_t, std::shared_ptr<SharedSurface>> sSurfaces;
static std::unordered_map<int64_t, std::shared_ptr<SharedTexture>> sTextures;
static std::unordered_map<int64_t, std::shared_ptr<std::thread>> sSurfaceThreads;
static std::unordered_map<int64_t, std::shared_ptr<std::thread>> sTextureThreads;
static std::unordered_map<int64_t, bool> sSurfaceThreadActive;
static std::unordered_map<int64_t, bool> sTextureThreadActive;
} // namespace Capture

namespace waffles {
CapturePlayer::CapturePlayer(ds::ui::SpriteEngine& g)
  : ds::ui::Sprite(g) {

	initDeviceResolutionMap();

	setTransparent(false);
	setSize(0.f, 0.f);
	setColor(ci::Color::white());
}

CapturePlayer::~CapturePlayer() {
	if (mCaptureId < 0 || Capture::sCaptures.find(mCaptureId) == Capture::sCaptures.end()) return;

	Capture::sCaptures[mCaptureId].users -= 1; // Decrement the active users
	if (Capture::sCaptures[mCaptureId].users <= 0) {
		// And if we were the last user, clean up after ourselves
		if (Capture::sSurfaceThreadActive[mCaptureId]) {
			Capture::sSurfaceThreadActive[mCaptureId] = false;
			Capture::sSurfaceThreads[mCaptureId]->join();
		}
		if (Capture::sTextureThreadActive[mCaptureId]) {
			Capture::sTextureThreadActive[mCaptureId] = false;
			Capture::sTextureThreads[mCaptureId]->join();
		}
		Capture::sSurfaces.erase(mCaptureId);
		Capture::sTextures.erase(mCaptureId);
		Capture::sCaptures.erase(mCaptureId);
	}
}

bool CapturePlayer::setCaptureSource(const std::string& sourceIdName) {

	bool goodCapture = false;
	auto idSource	 = ds::split(sourceIdName, ";", true);
	if (idSource.size() == 2) {
		goodCapture = setCaptureSource(ds::string_to_int(idSource[0]), idSource[1]);
	}

	if (!goodCapture) {
		goodCapture = setCaptureSourceWithUniqueName(sourceIdName);
	}

	if (goodCapture
		&& (Capture::sSurfaceThreadActive.find(mCaptureId) == Capture::sSurfaceThreadActive.end()
			|| !Capture::sSurfaceThreadActive[mCaptureId]
		   )
	   ) {
		auto fps = mEngine.getEngineSettings().getInt("devices:limit_thread_fps", 0, 0); // 0 = disabled
		Capture::sSurfaceThreads[mCaptureId] = std::shared_ptr<std::thread>(
			new std::thread(
				bind(
					&CapturePlayer::updateSurface,
					this,
					ci::gl::Context::create(ci::gl::context()),
					mCaptureId,
					fps
				)
			)
		);
		Capture::sTextureThreads[mCaptureId] = std::shared_ptr<std::thread>(
			new std::thread(
				bind(
					&CapturePlayer::updateTexture,
					this,
					ci::gl::Context::create(ci::gl::context()),
					mCaptureId,
					fps
				)
			)
		);
	}

	return goodCapture;
}

bool CapturePlayer::setCaptureSource(int id, const std::string& sourceName) {
	if (id < 0 || sourceName.empty()) return false;
	mCaptureId	= id;
	mSourceName = sourceName;

	if (Capture::sCaptures.find(mCaptureId) != Capture::sCaptures.end()) {
		// If we already have this source, just add ourself to the users
		Capture::sCaptures[mCaptureId].users += 1;
		setSize(Capture::sCaptures[mCaptureId].capture->getWidth(), Capture::sCaptures[mCaptureId].capture->getHeight());
	} else {
		// We haven't opened this source, try to
		try {
			for (auto&& dev : ci::Capture::getDevices(true)) {
				// Continue until we find our match
				if (dev->getUniqueId() != mCaptureId || dev->getName() != mSourceName) continue;

				auto res = ci::vec2(3840, 2160);
				if (mDeviceResolutionMap.find(sourceName) != mDeviceResolutionMap.end()) {
					res = mDeviceResolutionMap[sourceName];
				}
				Capture::sCaptures[mCaptureId].capture = ci::Capture::create(res.x, res.y, dev);
				break;
			}

			if (Capture::sCaptures[mCaptureId].capture) {
				Capture::sCaptures[mCaptureId].capture->start();
				Capture::sCaptures[mCaptureId].users = 1;

				setSize(Capture::sCaptures[mCaptureId].capture->getWidth(), Capture::sCaptures[mCaptureId].capture->getHeight());
			}
		} catch (const std::exception& e) {
			DS_LOG_WARNING("Unable to open capture device. ID: " << mCaptureId << ", Name: " << mSourceName);
			DS_LOG_WARNING("\tDevice not found or unavailable");
			return false;
		}
	}
	return true;
}

bool CapturePlayer::setCaptureSourceWithUniqueName(const std::string& uniqueName) {
	if (uniqueName.empty()) return false;
	mCaptureId	= (uint64_t)std::hash<std::string>{}(uniqueName);
	mSourceName = uniqueName;
	if (Capture::sCaptures.count(mCaptureId) && Capture::sCaptures[mCaptureId].capture) {
		// If we already have this source, just add ourself to the users
		Capture::sCaptures[mCaptureId].users += 1;
		setSize(Capture::sCaptures[mCaptureId].capture->getWidth(), Capture::sCaptures[mCaptureId].capture->getHeight());
	} else {
		// We haven't opened this source, try to
		try {
			for (auto&& dev : ci::Capture::getDevices(true)) {
				// Continue until we find our match
				if (dev->getName() != uniqueName) continue;
				
				auto res = ci::vec2(3840, 2160);
				if (mDeviceResolutionMap.find(uniqueName) != mDeviceResolutionMap.end()) {
					res = mDeviceResolutionMap[uniqueName];
				}
				Capture::sCaptures[mCaptureId].capture = ci::Capture::create(res.x, res.y, dev);
				break;
			}

			if (Capture::sCaptures[mCaptureId].capture) {
				Capture::sCaptures[mCaptureId].capture->start();
				Capture::sCaptures[mCaptureId].users = 1;

				setSize(Capture::sCaptures[mCaptureId].capture->getWidth(), Capture::sCaptures[mCaptureId].capture->getHeight());
			}
		} catch (const std::exception& e) {
			Capture::sCaptures.erase(mCaptureId);
			DS_LOG_WARNING("Unable to open capture device. Name: " << uniqueName);
			DS_LOG_WARNING("\tDevice not found or unavailable");
			return false;
		}
	}
	return true;
}

void CapturePlayer::drawLocalClient() {
	if (mCaptureId < 0 || Capture::sTextures.find(mCaptureId) == Capture::sTextures.end()) return;
	{
		std::scoped_lock lock(Capture::sTextures[mCaptureId]->mutex);
		ci::gl::draw(Capture::sTextures[mCaptureId]->texture, ci::Rectf(0.f, 0.f, getWidth(), getHeight()));
	}
}

void CapturePlayer::initDeviceResolutionMap() {
	// <setting name="devices:resolution_map" value="some_name:1920x1080, some_id:1280x720" type="string" />
	auto fullMapString = mEngine.getEngineSettings().getString("devices:resolution_map", 0, "");
	if (fullMapString.empty()) { return; }
	auto mapEntryStrings = ds::split(fullMapString, ",", true);
	for (auto mapEntry : mapEntryStrings) {
		auto nameResolutionStrings = ds::split(mapEntry, ":");
		if (nameResolutionStrings.size() != 2) { continue; }
		auto name = nameResolutionStrings[0];
		auto resolutionStrings = ds::split(nameResolutionStrings[1], "x");
		if (resolutionStrings.size() != 2) { continue; }
		auto resolution = ci::vec2(
			atoi(resolutionStrings[0].c_str()),
			atoi(resolutionStrings[1].c_str())
		);
		mDeviceResolutionMap[name] = resolution;
	}
}

void CapturePlayer::updateSurface(ci::gl::ContextRef context, uint64_t captureId, int fps) {
	if (captureId < 0) return;
	ci::ThreadSetup threadSetup;
	context->makeCurrent();
	Capture::sSurfaceThreadActive[captureId] = true;
	DS_LOG_INFO("Started CapturePlayer::updateSurface thread for " << captureId);
    auto interval = std::chrono::milliseconds(1000 / std::max(fps, 1));
    auto time = std::chrono::high_resolution_clock::now();
	Capture::sSurfaces[captureId] = std::make_shared<Capture::SharedSurface>();
    while (Capture::sSurfaceThreadActive[captureId]) {
		if (Capture::sCaptures.find(captureId) == Capture::sCaptures.end()
			|| !Capture::sCaptures[captureId].capture
			|| !Capture::sCaptures[captureId].capture->checkNewFrame())
		{
			continue;
		}
		{
			std::scoped_lock lock(Capture::sSurfaces[captureId]->mutex);
			Capture::sSurfaces[captureId]->surface = Capture::sCaptures[captureId].capture->getSurface();
        }
		time += interval;
        if (fps > 0) std::this_thread::sleep_until(time);
    }
	DS_LOG_INFO("Ended CapturePlayer::updateSurface thread for " << captureId);
}

void CapturePlayer::updateTexture(ci::gl::ContextRef context, uint64_t captureId, int fps) {
	if (captureId < 0) return;
	ci::ThreadSetup threadSetup;
	context->makeCurrent();
	std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO: no wait because good continue condition below
	Capture::sTextureThreadActive[captureId] = true;
	DS_LOG_INFO("Started CapturePlayer::updateTexture thread for " << captureId);
    auto interval = std::chrono::milliseconds(1000 / std::max(fps, 1));
    auto time = std::chrono::high_resolution_clock::now();
	Capture::sTextures[captureId] = std::make_shared<Capture::SharedTexture>();
    while (Capture::sTextureThreadActive[captureId]) {
		// TODO: some good if (condition) continue; to make sure there's a surface to work with
		ci::gl::Texture2dRef texture;
		{
			std::scoped_lock lock(Capture::sSurfaces[captureId]->mutex);
			texture = ci::gl::Texture::create(*Capture::sSurfaces[captureId]->surface);
		}
		{
			std::scoped_lock lock(Capture::sTextures[captureId]->mutex);
			Capture::sTextures[captureId]->texture = texture;
        }
		time += interval;
        if (fps > 0) std::this_thread::sleep_until(time);
    }
	DS_LOG_INFO("Ended CapturePlayer::updateTexture thread for " << captureId);
}


} // namespace waffles
