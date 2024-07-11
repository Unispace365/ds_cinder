#include "stdafx.h"

#include "capture_player.h"

#include <cinder/Capture.h>
#include <cinder/ip/Resize.h>

#include <ds/util/string_util.h>

namespace {
auto INIT = []() {
	ds::App::AddStartup([](ds::Engine& e) {
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
static std::unordered_map<int, cap> sCaptures;
} // namespace

namespace waffles {
CapturePlayer::CapturePlayer(ds::ui::SpriteEngine& g)
	: ds::ui::Sprite(g) {

	setTransparent(false);
	setSize(0.f, 0.f);
	setColor(ci::Color::white());
}

CapturePlayer::~CapturePlayer() {
	if (mCaptureId < 0 || sCaptures.find(mCaptureId) == sCaptures.end()) return;

	sCaptures[mCaptureId].users -= 1; // Decrement the active users
	if (sCaptures[mCaptureId].users <= 0) {
		// And if we were the last user, clean up after ourselves
		// sCaptures.erase(mCaptureId);
	}
}

void CapturePlayer::setCaptureSource(const std::string& sourceIdName) {
	auto idSource = ds::split(sourceIdName, ";");
	if (idSource.size() != 2) {
		DS_LOG_ERROR("Capture source needs to be in the format 'ID;NAME'");
		DS_LOG_ERROR("\tFailed Source: " << sourceIdName);
		return;
	}

	setCaptureSource(ds::string_to_int(idSource[0]), idSource[1]);
}

void CapturePlayer::setCaptureSource(int id, const std::string& sourceName) {
	if (id < 0 || sourceName.empty()) return;

	mCaptureId	= id;
	mSourceName = sourceName;

	if (sCaptures.find(mCaptureId) != sCaptures.end()) {
		// If we already have this source, just add ourself to the users
		sCaptures[mCaptureId].users += 1;
		// setSize(sCaptures[mCaptureId].capture->getWidth(), sCaptures[mCaptureId].capture->getHeight());
	} else {
		// We haven't opened this source, try to
		try {
			for (auto&& dev : ci::Capture::getDevices(true)) {
				DS_LOG_INFO(dev->getUniqueId() << ";" << dev->getName());
				// Continue until we find our match
				if (dev->getUniqueId() != mCaptureId || dev->getName() != mSourceName) continue;

				sCaptures[mCaptureId].capture = ci::Capture::create(3840, 2160, dev);
				break;
			}

			if (sCaptures[mCaptureId].capture) {
				sCaptures[mCaptureId].capture->start();
				sCaptures[mCaptureId].users = 1;

				// setSize(sCaptures[mCaptureId].capture->getWidth(), sCaptures[mCaptureId].capture->getHeight());
			}
		} catch (const std::exception& e) {
			DS_LOG_WARNING("Unable to open capture device. ID: " << mCaptureId << ", Name: " << mSourceName);
			DS_LOG_WARNING("\tDevice not found or unavailable");
		}
	}
}

void CapturePlayer::onUpdateServer(const ds::UpdateParams& up) {
	if (mCaptureId < 0 || !sCaptures[mCaptureId].capture || !sCaptures[mCaptureId].capture->checkNewFrame()) return;

	// If we have a new frame, save it to the texture
	sCaptures[mCaptureId].texture = ci::gl::Texture::create(*sCaptures[mCaptureId].capture->getSurface());
}

void CapturePlayer::drawLocalClient() {
	if (mCaptureId < 0 || !sCaptures[mCaptureId].texture) return;

	auto inBox =
		ci::Rectf(0.f, 0.f, sCaptures[mCaptureId].texture->getWidth(), sCaptures[mCaptureId].texture->getHeight());
	auto outBox = ci::Rectf(0.f, 0.f, getWidth(), getHeight());

	// ci::gl::draw(sCaptures[mCaptureId].texture, ci::Area(outBox.getCenteredFit(inBox, true)), outBox);

	auto	 texRect = outBox.getCenteredFit(inBox, true);
	ci::vec2 uv1, uv2;
	// things are backwards sometimes - upper left , lower right is opposite
	uv1.x = texRect.getX2() / inBox.getWidth();
	uv1.y = texRect.getY2() / inBox.getHeight();
	uv2.x = texRect.getX1() / inBox.getWidth();
	uv2.y = texRect.getY1() / inBox.getHeight();

	ci::gl::ScopedTextureBind scTex{sCaptures[mCaptureId].texture};
	ci::gl::ScopedGlslProg	  scGlsl{ci::gl::getStockShader(ci::gl::ShaderDef().color().texture())};
	ci::gl::drawSolidRoundedRect(outBox, getCornerRadius(), 0, uv1, uv2);
}

void CapturePlayer::saveImage() {
	auto inBox =
		ci::Rectf(0.f, 0.f, sCaptures[mCaptureId].texture->getWidth(), sCaptures[mCaptureId].texture->getHeight());
	auto outBox = ci::Rectf(0.f, 0.f, getWidth(), getHeight());

	// ci::gl::draw(sCaptures[mCaptureId].texture, ci::Area(outBox.getCenteredFit(inBox, true)), outBox);

	auto texRect = outBox.getCenteredFit(inBox, true);

	std::string	   localPath = ds::Environment::expand("%LOCAL%/waffles-pics/captured.png");
	ci::Surface	   surface(sCaptures[mCaptureId].texture->createSource());
	ci::SurfaceRef outSurface = ci::Surface::create(int(texRect.getWidth()), int(texRect.getHeight()), false);
	ci::ip::resize(surface, ci::Area(texRect), &*outSurface, ci::Area(0, 0, texRect.getWidth(), texRect.getHeight()));
	ci::writeImage(localPath, *outSurface);
}

} // namespace mv