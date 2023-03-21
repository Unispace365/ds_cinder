#include "sync_video_player_app.h"

#include <cinder/app/RendererGl.h>

#include <Poco/String.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "ds/ui/sprite/gst_video.h"
#include <ds/ui/sprite/panoramic_video.h>

namespace ds {

sync_video_player::sync_video_player()
  : inherited(ds::RootList().ortho().pickColor())
  , mGlobals(mEngine)
  , mVideo(nullptr)
  , mVideoHolder(nullptr)
  , mFpsDisplay(nullptr)
  , mVerbose(false)
  , mSeekSpeed(1.0)
  , mVsb(nullptr) {


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
}

void sync_video_player::setupServer() {

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mEngine.getRootSprite(0).clearChildren();

	mVsb   = nullptr;
	mVideo = nullptr;

	ds::ui::Sprite& rootSprite = mEngine.getRootSprite();

	ds::ui::Sprite* spriggy = new ds::ui::Sprite(mEngine, mEngine.getWorldWidth(), mEngine.getWorldHeight());
	spriggy->setTransparent(false);
	spriggy->setColor(ci::Color(0.05f, 0.05f, 0.2f));
	rootSprite.addChildPtr(spriggy);

	mVideoHolder = new ds::ui::Sprite(mEngine);
	rootSprite.addChildPtr(mVideoHolder);

	mFpsDisplay = new ds::ui::Text(mEngine);
	mFpsDisplay->setFont("noto-bold", 24.0f);
	mFpsDisplay->setColor(ci::Color::white());
	mFpsDisplay->setText("GstFps");
	mFpsDisplay->setPosition(mEngine.getWorldWidth() / 4.0f, 30.0f);
	rootSprite.addChildPtr(mFpsDisplay);

	// loadVideo("%APP%/data/video/test_video.mp4");
}

void sync_video_player::fileDrop(ci::app::FileDropEvent event) {
	if (mEngine.getMode() == ds::ui::SpriteEngine::CLIENT_MODE) {
		return;
	}
	std::vector<std::string> paths;
	for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
		paths.push_back((*it).string());
	}

	if (paths.empty()) {
		return;
	}
#if 0
	auto mPanoramicVideo = new ds::ui::PanoramicVideo(mEngine);
	mEngine.getRootSprite().addChildPtr(mPanoramicVideo);
	//mPanoramicVideo->setSize(1366.0, 768.0f);
	mPanoramicVideo->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mPanoramicVideo->setCenter(0.5f, 0.5f);
	mPanoramicVideo->setPosition(0.5f * mEngine.getWorldWidth(), 0.5f * mEngine.getWorldHeight());
	mPanoramicVideo->loadVideo(paths.front());
	mPanoramicVideo->setFOV(60.0f);

	return;
#endif

	loadVideo(paths.front());
}

void sync_video_player::loadVideo(const std::string& videoPath) {

	mVideo = new ds::ui::GstVideo(mEngine);
	mVideo->generateAudioBuffer(true);
	mVideo->setLooping(true);
	mVideo->setCheckBounds(false);
	// mVideo->setVerboseLogging(mVerbose);
	mVideo->enable(true);

	std::vector<std::string> allowedInstances;
	allowedInstances.push_back("debug_client");
	// mVideo->setPlayableInstances(allowedInstances);

	mVideo->setAutoSynchronize(true);

	mVideo->loadVideo(videoPath);

	mVideo->enableMultiTouch(ds::ui::MULTITOUCH_CAN_SCALE | ds::ui::MULTITOUCH_CAN_POSITION);
	mVideo->setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos) {
		ds::ui::GstVideo* video = dynamic_cast<ds::ui::GstVideo*>(bs);
		if (video) {
			if (mSelectedVideo == video) {
				if (video->getIsPlaying()) {
					video->pause();
					mVsb->mPauseOn->show();
					mVsb->mPauseOff->hide();
				} else {
					video->play();
					mVsb->mPauseOn->hide();
					mVsb->mPauseOff->show();
				}
			} else {
				mSelectedVideo = video;
				video->sendToFront();
				updateScrubBar(mSelectedVideo);
			}
		}
	});
	mLoadedVideos.push_back(mVideo);
	mVideoHolder->addChildPtr(mVideo);
	mSelectedVideo = mVideo;

	if (!mVsb) {
		setupScrubBar();
	}
	if (mVsb) {
		mVsb->linkVideo(mVideo);
		mVsb->sendToFront();
	}
}
void sync_video_player::setupScrubBar() {

	// Setup Scrub bar

	mVsb = new VideoScrubBar(mGlobals, 800.0f);
	mVideoHolder->addChildPtr(mVsb);
	mVsb->setCenter(0.5f, 0.5f);
	mVsb->setPosition(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() - 100.0f);
	mVsb->show();
	mVsb->setOpacity(1.0f);
	mVsb->mHome->hide();

	mVsb->mPauseOff->setClickFn([this] {
		if (mSelectedVideo) {
			mSelectedVideo->pause();
			mVsb->mPauseOn->show();
			mVsb->mPauseOff->hide();
		}
	});

	mVsb->mPauseOn->setClickFn([this] {
		if (mSelectedVideo) {

			mSelectedVideo->play();
			mVsb->mPauseOn->hide();
			mVsb->mPauseOff->show();
		}
	});

	mVsb->mVolumeOn->setClickFn([this] {
		if (mSelectedVideo) {

			mSelectedVideo->setMute(true);
			mVsb->mVolumeOff->show();
			mVsb->mVolumeOn->hide();
		}
	});

	mVsb->mVolumeOff->setClickFn([this] {
		if (mSelectedVideo) {

			mSelectedVideo->setMute(false);
			mVsb->mVolumeOff->hide();
			mVsb->mVolumeOn->show();
		}
	});
}

void sync_video_player::update() {
	inherited::update();

	if (mVideo) {
		mPan = mGlobals.getSettingsLayout().getFloat("pan:value", 0, 0.0f);
		mVideo->setPan(mPan);
		std::stringstream ss;
		ss << "fps: " << mVideo->getVideoPlayingFramerate();
	}
}

void sync_video_player::updateScrubBar(ds::ui::Video* video) {
	if (!mVsb || !video) return;
	mVsb->linkVideo(video);

	if (!video->getIsMuted()) {
		mVsb->mVolumeOff->hide();
		mVsb->mVolumeOn->show();
	} else {
		mVsb->mVolumeOff->show();
		mVsb->mVolumeOn->hide();
	}

	if (video->getCurrentStatus() == ds::ui::GstVideo::Status::STATUS_PAUSED) {
		mVsb->mPauseOn->show();
		mVsb->mPauseOff->hide();
	} else {
		mVsb->mPauseOn->hide();
		mVsb->mPauseOff->show();
	}

	mVsb->sendToFront();
}

void sync_video_player::onKeyDown(ci::app::KeyEvent event) {
	using ci::app::KeyEvent;

	if (event.isControlDown() && event.getChar() == KeyEvent::KEY_l) {
		if (mVideo) {
			mVideo->setPan(-1.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_c) {
		if (mVideo) {
			mVideo->setPan(0.0f);
		}
	} else if (event.isControlDown() && event.getChar() == KeyEvent::KEY_r) {
		if (mVideo) {
			mVideo->setPan(1.0f);
		}
	} else if (event.isControlDown() &&
			   event.getChar() == KeyEvent::KEY_d) { // d = delete selected video (only on master).
		if (mEngine.getMode() == ds::ui::SpriteEngine::CLIENT_MODE) {
			return;
		}

		if (mSelectedVideo) {
			// TODO: Need to schedule this to be removed,and actually remove during update.
			mLoadedVideos.erase(std::remove(mLoadedVideos.begin(), mLoadedVideos.end(), mSelectedVideo),
								mLoadedVideos.end());
			mSelectedVideo->release();
			mVsb->linkVideo(nullptr);
			if (!mLoadedVideos.empty()) {
				mSelectedVideo = mLoadedVideos[0];
				updateScrubBar(mSelectedVideo);
				mVsb->sendToFront();
			} else {
				mSelectedVideo = nullptr;
			}
		}
	}
}

} // namespace ds

// This line tells Cinder to actually create the application
CINDER_APP(ds::sync_video_player, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })
