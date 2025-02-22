#include "stdafx.h"

#include "capture_player.h"

#include <cinder/Capture.h>

#include <ds/util/string_util.h>

#include <thread>
#include <chrono>

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
	ci::gl::TextureRef texture;
	int				   users = 0;
};

// Holds all open captures, allowing us to display multiple copies of a capture while only using one GPU resource
// The first CapturePlayer to update for a given Capture source will update the texture
static std::unordered_map<int64_t, cap>		sCaptures;
static std::unordered_map<std::string, cap> sUNCaptures;
static std::unordered_map<int64_t, cinder::Surface8uRef> sSurfaces;
static std::unordered_map<int64_t, bool> sThreadActive;

void updateTexture(uint64_t captureId, int fps) {
	if (captureId < 0) return;
	sThreadActive[captureId] = true;
	DS_LOG_INFO("Started CapturePlayer::updateTexture thread for " << captureId);
    auto interval = std::chrono::milliseconds(1000 / fps);
    auto time = std::chrono::high_resolution_clock::now();
    while (sThreadActive[captureId]) {
		if (Capture::sCaptures.find(captureId) == Capture::sCaptures.end() ||
			!Capture::sCaptures[captureId].capture || !Capture::sCaptures[captureId].capture->checkNewFrame()) continue;
		sSurfaces[captureId] = Capture::sCaptures[captureId].capture->getSurface();
        time += interval;
        std::this_thread::sleep_until(time);
    }
	DS_LOG_INFO("Ended CapturePlayer::updateTexture thread for " << captureId);
}
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

	if (Capture::sThreadActive[mCaptureId]) {
		Capture::sThreadActive[mCaptureId] = false;
		mUpdateTextureThread.join();
	}

	Capture::sCaptures[mCaptureId].users -= 1; // Decrement the active users
	if (Capture::sCaptures[mCaptureId].users <= 0) {
		// And if we were the last user, clean up after ourselves
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

	if (goodCapture) {
		auto updateTextureFunction = [this] { Capture::updateTexture(mCaptureId, 60); }; // TODO: dynamic fps
		mUpdateTextureThread = std::thread(updateTextureFunction);
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
	if (mCaptureId < 0 || Capture::sSurfaces.find(mCaptureId) == Capture::sSurfaces.end()) return;
	auto texture = ci::gl::Texture::create(*Capture::sSurfaces[mCaptureId]);
	ci::gl::draw(texture, ci::Rectf(0.f, 0.f, getWidth(), getHeight()));
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

} // namespace waffles
