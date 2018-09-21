#include "stdafx.h"

#include "media_player.h"

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include <ds/util/file_meta_data.h>
#include "ds/ui/media/player/panoramic_video_player.h"
#include "ds/ui/media/player/pdf_player.h"
#include "ds/ui/media/player/stream_player.h"
#include "ds/ui/media/player/video_player.h"
#include "ds/ui/media/player/web_player.h"

#include "ds/ui/media/interface/pdf_interface.h"
#include "ds/ui/media/interface/video_interface.h"
#include "ds/ui/media/interface/web_interface.h"
#include "ds/ui/media/media_interface.h"

#include "ds/ui/sprite/gst_video.h"
#include "ds/ui/sprite/panoramic_video.h"
#include "ds/ui/sprite/web.h"

namespace {
class Init {
  public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("media_player", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
				return new ds::ui::MediaPlayer(enginey, true);
			});

			e.registerSpritePropertySetter(
				"media_player_src", [](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer) {
					if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
						ds::Resource theResource;
						int			 mediaType = ds::Resource::parseTypeFromFilename(theValue);
						if (mediaType == ds::Resource::WEB_TYPE) {
							theResource = ds::Resource(theValue, mediaType);
						} else {
							std::string absPath = ds::filePathRelativeTo(fileReferrer, theValue);
							theResource			= ds::Resource(absPath);
						}
						mediaPlayer->loadMedia(theResource, true);
					} else {
						DS_LOG_WARNING("Tried to set the property media_player_src on a non-mediaPlayer sprite");
						return;
					}
				});

			e.registerSpritePropertySetter("media_player_auto_start", [](ds::ui::Sprite& theSprite, const std::string& theValue,
																		 const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs					 = mediaPlayer->getSettings();
					mvs.mVideoAutoPlayFirstFrame = !ds::parseBoolean(theValue);
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_auto_start on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter(
				"media_player_show_interface",
				[](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer) {
					if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
						auto& mvs				  = mediaPlayer->getSettings();
						mvs.mShowInterfaceAtStart = ds::parseBoolean(theValue);
						mediaPlayer->setSettings(mvs);
					} else {
						DS_LOG_WARNING("Tried to set the property media_player_show_interface on a non-mediaPlayer sprite");
						return;
					}
				});

			e.registerSpritePropertySetter("media_player_web_size", [](ds::ui::Sprite& theSprite, const std::string& theValue,
																	   const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs			= mediaPlayer->getSettings();
					mvs.mWebDefaultSize = ci::vec2(ds::parseVector(theValue));
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_web_size on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter("media_player_letterbox", [](ds::ui::Sprite& theSprite, const std::string& theValue,
																		const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs	  = mediaPlayer->getSettings();
					mvs.mLetterBox = ds::parseBoolean(theValue);
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_letterbox on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter("media_player_video_volume", [](ds::ui::Sprite& theSprite, const std::string& theValue,
																		   const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs		 = mediaPlayer->getSettings();
					mvs.mVideoVolume = ds::string_to_float(theValue);
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_video_volume on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter("media_player_video_loop", [](ds::ui::Sprite& theSprite, const std::string& theValue,
																		 const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs	  = mediaPlayer->getSettings();
					mvs.mVideoLoop = ds::parseBoolean(theValue);
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_video_loop on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter("media_player_video_reset_on_complete", [](ds::ui::Sprite&	theSprite,
																					  const std::string& theValue,
																					  const std::string& fileReferrer) {
				if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
					auto& mvs				  = mediaPlayer->getSettings();
					mvs.mVideoResetOnComplete = ds::parseBoolean(theValue);
					mediaPlayer->setSettings(mvs);
				} else {
					DS_LOG_WARNING("Tried to set the property media_player_video_reset_on_complete on a non-mediaPlayer sprite");
					return;
				}
			});

			e.registerSpritePropertySetter(
				"media_player_standard_click",
				[](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer) {
					if (auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) {
						if (ds::parseBoolean(theValue)) {
							mediaPlayer->enableStandardClick();
						}
					} else {
						DS_LOG_WARNING("Tried to set the property media_player_standard_click on a non-mediaPlayer sprite");
						return;
					}
				});
		});
	}
};

Init INIT;
}  // namespace


namespace ds {
namespace ui {

MediaPlayer::MediaPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
  : ds::ui::Sprite(eng)
  , mEmbedInterface(embedInterface) {
	setDefaultProperties();
}

MediaPlayer::MediaPlayer(ds::ui::SpriteEngine& eng, const std::string& mediaPath, const bool embedInterface)
  : ds::ui::Sprite(eng)
  , mResource(mediaPath, ds::Resource::parseTypeFromFilename(mediaPath))
  , mEmbedInterface(embedInterface) {
	setDefaultProperties();
}

MediaPlayer::MediaPlayer(ds::ui::SpriteEngine& eng, const ds::Resource& resource, const bool embedInterface)
  : ds::ui::Sprite(eng)
  , mResource(resource)
  , mEmbedInterface(embedInterface) {
	setDefaultProperties();
}


void MediaPlayer::setSettings(const MediaViewerSettings& newSettings) { mMediaViewerSettings = newSettings; }

void MediaPlayer::setDefaultProperties() {
	mAnimDuration		= 0.35f;
	mContentAspectRatio = 1.0;
	mLayoutFixedAspect  = true;
	setDefaultBounds(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	setWebViewSize(ci::vec2(0.0f, 0.0f));
	setCacheImages(false);
}

void MediaPlayer::loadMedia(const std::string& mediaPath, const bool initializeImmediately) {
	loadMedia(ds::Resource(mediaPath, ds::Resource::parseTypeFromFilename(mediaPath)), initializeImmediately);
}

void MediaPlayer::loadMedia(const ds::Resource& reccy, const bool initializeImmediately) {
	if (mInitialized) uninitialize();

	mResource = reccy;

	if (initializeImmediately) initialize();
}


void MediaPlayer::setDefaultBounds(const float defaultWidth, const float defaultHeight) {
	mMediaViewerSettings.mDefaultBounds.x = defaultWidth;
	mMediaViewerSettings.mDefaultBounds.y = defaultHeight;
}

void MediaPlayer::setWebViewSize(const ci::vec2 webSize) { mMediaViewerSettings.mWebDefaultSize = webSize; }

void MediaPlayer::initialize() {
	if (mInitialized) return;

	const int mediaType = mResource.getType();
	if (mediaType == ds::Resource::ERROR_TYPE || mediaType == ds::Resource::FONT_TYPE) {
		if (!mResource.empty()) {
			DS_LOG_WARNING("Whoopsies - tried to open a media player on an invalid file type. "
						   << mResource.getAbsoluteFilePath());
		}
		return;
	}

	// do this first to avoid recursion problems
	mInitialized = true;

	mContentAspectRatio = 1.0f;

	bool showThumbnail = true;

	if (mediaType == ds::Resource::IMAGE_TYPE) {
		initializeImage();
	} else if (mediaType == ds::Resource::VIDEO_TYPE) {
		initializeVideo();
	} else if (mediaType == ds::Resource::VIDEO_PANORAMIC_TYPE) {
		initializeVideoPanoramic();
	} else if (mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
		initializeVideoStream();
	} else if (mediaType == ds::Resource::PDF_TYPE) {
		initializePdf();
		showThumbnail = false;
	} else if (mediaType == ds::Resource::WEB_TYPE) {
		initializeWeb();
	} else {
		DS_LOG_WARNING("Whoopsies - tried to open a media player on an invalid file type. "
					   << mResource.getAbsoluteFilePath() << " " << ds::utf8_from_wstr(mResource.getTypeName()));
	}

	if (showThumbnail && (mResource.getThumbnailId() > 0 || !mResource.getThumbnailFilePath().empty())) {
		initializeThumbnail();
	}

	if (mInitializedCallback) {
		mInitializedCallback();
	}
}

void MediaPlayer::initializeThumbnail() {
	int flags = 0;
	if (mMediaViewerSettings.mCacheImages) {
		flags |= Image::IMG_CACHE_F;
	}
	mThumbnailImage = new ds::ui::Image(mEngine);
	addChildPtr(mThumbnailImage);
	mThumbnailImage->sendToBack();
	if (!mResource.getThumbnailFilePath().empty()) {
		mThumbnailImage->setImageFile(mResource.getThumbnailFilePath(), flags);
	} else {
		mThumbnailImage->setImageResource(mResource.getThumbnailId(), flags);
	}
	mThumbnailImage->setOpacity(0.0f);
	mThumbnailImage->setStatusCallback([this](ds::ui::Image::Status status) {
		if (status.mCode == status.STATUS_LOADED && mThumbnailImage && mPrimaryImage && !mPrimaryImage->isLoaded()) {
			mThumbnailImage->tweenOpacity(1.0f, mAnimDuration);
		}
	});
}

void MediaPlayer::initializeImage() {
	// Depends on base initialize to check already initialized case
	int flags = Image::IMG_ENABLE_MIPMAP_F;
	if (mMediaViewerSettings.mCacheImages) {
		flags |= Image::IMG_CACHE_F;
	}
	mPrimaryImage = new ds::ui::Image(mEngine);
	addChildPtr(mPrimaryImage);

	mPrimaryImage->setOpacity(0.0f);
	mPrimaryImage->setStatusCallback([this](ds::ui::Image::Status status) {
		if (status.mCode == status.STATUS_LOADED && mPrimaryImage) {
			mPrimaryImage->tweenOpacity(1.0f, mAnimDuration);
			if (mThumbnailImage) {
				mThumbnailImage->tweenOpacity(0.0f, mAnimDuration);
			}
			if (mStatusCallback) {
				mStatusCallback(true);
			}
		}
	});

	mPrimaryImage->setImageResource(mResource, flags);

	mContentAspectRatio = mPrimaryImage->getWidth() / mPrimaryImage->getHeight();

	setSizeAll(mPrimaryImage->getSize());
}

void MediaPlayer::initializeVideo() {
	// Depends on base initialize to check already initialized case
	mVideoPlayer = new VideoPlayer(mEngine, mEmbedInterface);
	addChildPtr(mVideoPlayer);

	mVideoPlayer->setErrorCallback([this](const std::string& msg) {
		if (mErrorCallback) mErrorCallback(msg);
	});

	mVideoPlayer->setGoodStatusCallback([this] {
		if (mStatusCallback) mStatusCallback(true);
	});

	mVideoPlayer->setPan(mMediaViewerSettings.mVideoPanning);
	mVideoPlayer->setVolume(mMediaViewerSettings.mVideoVolume);
	mVideoPlayer->setAutoSynchronize(mMediaViewerSettings.mVideoAutoSync);
	mVideoPlayer->setPlayableInstances(mMediaViewerSettings.mVideoPlayableInstances);
	mVideoPlayer->setAutoPlayFirstFrame(mMediaViewerSettings.mVideoAutoPlayFirstFrame);
	mVideoPlayer->setVideoLoop(mMediaViewerSettings.mVideoLoop);
	mVideoPlayer->setResetOnVideoComplete(mMediaViewerSettings.mVideoResetOnComplete);
	mVideoPlayer->setShowInterfaceAtStart(mMediaViewerSettings.mShowInterfaceAtStart);
	mVideoPlayer->setAudioDevices(mMediaViewerSettings.mVideoAudioDevices);
	mVideoPlayer->setLetterbox(mMediaViewerSettings.mLetterBox);

	mVideoPlayer->setMedia(mResource.getPortableFilePath());

	mContentAspectRatio = mVideoPlayer->getWidth() / mVideoPlayer->getHeight();
	setSizeAll(mVideoPlayer->getSize());
}

void MediaPlayer::initializeVideoPanoramic() {
	// Depends on base initialize to check already initialized case
	mPanoramicPlayer = new PanoramicVideoPlayer(mEngine, mEmbedInterface);
	addChildPtr(mPanoramicPlayer);

	mPanoramicPlayer->setErrorCallback([this](const std::string& msg) {
		if (mErrorCallback) mErrorCallback(msg);
	});

	mPanoramicPlayer->setGoodStatusCallback([this] {
		if (mStatusCallback) mStatusCallback(true);
	});

	mPanoramicPlayer->setPan(mMediaViewerSettings.mVideoPanning);
	mPanoramicPlayer->setVolume(mMediaViewerSettings.mVideoVolume);
	mPanoramicPlayer->setAutoSynchronize(mMediaViewerSettings.mVideoAutoSync);
	mPanoramicPlayer->setPlayableInstances(mMediaViewerSettings.mVideoPlayableInstances);
	mPanoramicPlayer->setAutoPlayFirstFrame(mMediaViewerSettings.mVideoAutoPlayFirstFrame);
	mPanoramicPlayer->setVideoLoop(mMediaViewerSettings.mVideoLoop);
	mPanoramicPlayer->setShowInterfaceAtStart(mMediaViewerSettings.mShowInterfaceAtStart);
	mPanoramicPlayer->setAudioDevices(mMediaViewerSettings.mVideoAudioDevices);

	mPanoramicPlayer->setMedia(mResource.getPortableFilePath());

	if (!mMediaViewerSettings.mPanoramicVideoInteractive) {
		auto pvs = mPanoramicPlayer->getPanoramicVideo();
		if (pvs) {
			pvs->enable(false);
		}
	}

	mContentAspectRatio = mPanoramicPlayer->getWidth() / mPanoramicPlayer->getHeight();
	setSizeAll(mPanoramicPlayer->getSize());
}

void MediaPlayer::initializeVideoStream() {
	// Depends on base initialize to check already initialized case
	mStreamPlayer = new StreamPlayer(mEngine, mEmbedInterface);
	addChildPtr(mStreamPlayer);

	mStreamPlayer->setErrorCallback([this](const std::string& msg) {
		if (mErrorCallback) mErrorCallback(msg);
	});

	mStreamPlayer->setGoodStatusCallback([this] {
		if (mStatusCallback) mStatusCallback(true);
	});

	mStreamPlayer->setVolume(mMediaViewerSettings.mVideoVolume);
	mStreamPlayer->setShowInterfaceAtStart(mMediaViewerSettings.mShowInterfaceAtStart);
	mStreamPlayer->setLetterbox(mMediaViewerSettings.mLetterBox);
	mStreamPlayer->setStreamLatency(mMediaViewerSettings.mVideoStreamingLatency);
	mStreamPlayer->setResource(mResource);

	mContentAspectRatio = mStreamPlayer->getWidth() / mStreamPlayer->getHeight();
	setSizeAll(mStreamPlayer->getSize());
}

void MediaPlayer::initializePdf() {
	// Depends on base initialize to check already initialized case
	mPDFPlayer = new PDFPlayer(mEngine, mEmbedInterface, mMediaViewerSettings.mPdfCacheNextPrev);
	addChildPtr(mPDFPlayer);

	mPDFPlayer->setLetterbox(mMediaViewerSettings.mLetterBox);
	mPDFPlayer->setShowInterfaceAtStart(mMediaViewerSettings.mShowInterfaceAtStart);
	mPDFPlayer->setResource(mResource);

	mPDFPlayer->setErrorCallback([this](const std::string& msg) {
		if (mErrorCallback) mErrorCallback(msg);
	});

	mPDFPlayer->setGoodStatusCallback([this] {
		if (mStatusCallback) {
			mStatusCallback(true);
		}
	});

	mPDFPlayer->setSizeChangedCallback([this](const ci::vec2& newSize) {
		// setSize(mPDFPlayer->getWidth(), mPDFPlayer->getHeight());
		// change this size here?
		if (mMediaSizeChangedCallback) {
			mContentAspectRatio = newSize.x / newSize.y;
			mMediaSizeChangedCallback(newSize);
		} else {
			mPDFPlayer->layout();
		}
	});


	mContentAspectRatio = mPDFPlayer->getWidth() / mPDFPlayer->getHeight();
	setSizeAll(mPDFPlayer->getSize());
}

void MediaPlayer::initializeWeb() {
	// Depends on base initialize to check already initialized case
	mWebPlayer = new WebPlayer(mEngine, mEmbedInterface);
	addChildPtr(mWebPlayer);
	mWebPlayer->setWebViewSize(mMediaViewerSettings.mWebDefaultSize);
	mWebPlayer->setLetterbox(mMediaViewerSettings.mLetterBox);
	mWebPlayer->setKeyboardParams(mMediaViewerSettings.mWebKeyboardKeyScale, mMediaViewerSettings.mWebAllowKeyboard,
								  mMediaViewerSettings.mWebKeyboardAbove);
	mWebPlayer->setAllowTouchToggle(mMediaViewerSettings.mWebAllowTouchToggle);
	mWebPlayer->setShowInterfaceAtStart(mMediaViewerSettings.mShowInterfaceAtStart);
	mWebPlayer->setStartInteractable(mMediaViewerSettings.mWebStartTouchable);

	mWebPlayer->setMedia(mResource.getAbsoluteFilePath());

	if (mWebPlayer->getWeb()) {
		mWebPlayer->getWeb()->setDocumentReadyFn([this] {
			if (mWebPlayer->getWebInterface()) {
				mWebPlayer->getWebInterface()->updateWidgets();
			}
			if (mStatusCallback) mStatusCallback(true);
		});
		mWebPlayer->getWeb()->setErrorCallback([this](const std::string& errorMsg) {
			if (mErrorCallback) mErrorCallback(errorMsg);
		});
	}

	mContentAspectRatio = mWebPlayer->getWidth() / mWebPlayer->getHeight();
	setSizeAll(mWebPlayer->getSize());
}

void MediaPlayer::uninitialize() {
	if (!mInitialized) return;

	if (mThumbnailImage) mThumbnailImage->release();
	if (mVideoPlayer) mVideoPlayer->release();
	if (mPanoramicPlayer) mPanoramicPlayer->release();
	if (mStreamPlayer) mStreamPlayer->release();
	if (mPDFPlayer) mPDFPlayer->release();
	if (mPrimaryImage) mPrimaryImage->release();
	if (mWebPlayer) mWebPlayer->release();

	mThumbnailImage  = nullptr;
	mVideoPlayer	 = nullptr;
	mPanoramicPlayer = nullptr;
	mPDFPlayer		 = nullptr;
	mPrimaryImage	= nullptr;
	mWebPlayer		 = nullptr;

	mInitialized = false;
}

void MediaPlayer::onSizeChanged() { layout(); }

void MediaPlayer::layout() {

	if (mVideoPlayer) {
		mVideoPlayer->setSize(getWidth(), getHeight());
	}

	if (mPanoramicPlayer) {
		mPanoramicPlayer->setSize(getWidth(), getHeight());
	}

	if (mStreamPlayer) {
		mStreamPlayer->setSize(getWidth(), getHeight());
	}

	if (mPDFPlayer) {
		mPDFPlayer->setSize(getWidth(), getHeight());
	}

	if (mThumbnailImage) {
		fitInside(mThumbnailImage, ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), mMediaViewerSettings.mLetterBox);
	}

	if (mPrimaryImage) {
		fitInside(mPrimaryImage, ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), mMediaViewerSettings.mLetterBox);
	}

	if (mWebPlayer) {
		mWebPlayer->setSize(getWidth(), getHeight());
	}

	onLayout();
}

void MediaPlayer::enter() {
	initialize();

	if (mVideoPlayer) mVideoPlayer->play();
	if (mPanoramicPlayer) mPanoramicPlayer->play();
}

void MediaPlayer::exit() {
	if (mVideoPlayer) mVideoPlayer->pause();
	if (mPanoramicPlayer) mPanoramicPlayer->pause();
}

void MediaPlayer::userInputReceived() {
	ds::ui::Sprite::userInputReceived();

	showInterface();
}

void MediaPlayer::showInterface() {
	if (mVideoPlayer) mVideoPlayer->showInterface();
	if (mPanoramicPlayer) mPanoramicPlayer->showInterface();
	if (mStreamPlayer) mStreamPlayer->showInterface();
	if (mPDFPlayer) mPDFPlayer->showInterface();
	if (mWebPlayer) mWebPlayer->showInterface();
}

void MediaPlayer::hideInterface() {
	if (mVideoPlayer) mVideoPlayer->hideInterface();
	if (mPanoramicPlayer) mPanoramicPlayer->hideInterface();
	if (mStreamPlayer) mStreamPlayer->hideInterface();
	if (mPDFPlayer) mPDFPlayer->hideInterface();
	if (mWebPlayer) mWebPlayer->hideInterface();
}

void MediaPlayer::stopContent() {
	if (mVideoPlayer) mVideoPlayer->stop();
	if (mPanoramicPlayer) mPanoramicPlayer->stop();
	if (mStreamPlayer) mStreamPlayer->stop();
}

void MediaPlayer::playContent() {
	if (mVideoPlayer) mVideoPlayer->play();
	if (mPanoramicPlayer) mPanoramicPlayer->play();
	if (mStreamPlayer) mStreamPlayer->play();
}

void MediaPlayer::pauseContent() {
	if (mVideoPlayer) mVideoPlayer->pause();

	if (mPanoramicPlayer) mPanoramicPlayer->pause();

	if (mStreamPlayer) mStreamPlayer->pause();
}

void MediaPlayer::togglePlayPause() {
	if (mVideoPlayer) mVideoPlayer->togglePlayPause();
	if (mPanoramicPlayer) mPanoramicPlayer->togglePlayPause();
	if (mStreamPlayer) mStreamPlayer->togglePlayPause();
}

void MediaPlayer::toggleMute() {
	if (mVideoPlayer) mVideoPlayer->toggleMute();
	if (mPanoramicPlayer) mPanoramicPlayer->toggleMute();
	if (mStreamPlayer) mStreamPlayer->toggleMute();
}

ds::ui::Sprite* MediaPlayer::getPlayer() {
	if (mVideoPlayer) return mVideoPlayer;
	if (mPanoramicPlayer) return mPanoramicPlayer;
	if (mPDFPlayer) return mPDFPlayer;
	if (mStreamPlayer) return mStreamPlayer;
	if (mWebPlayer) return mWebPlayer;
	if (mPrimaryImage) return mPrimaryImage;
	return nullptr;
}

ds::ui::MediaInterface* MediaPlayer::getMediaInterface() {
	if (mVideoPlayer) return mVideoPlayer->getVideoInterface();
	if (mPanoramicPlayer) return mPanoramicPlayer->getVideoInterface();
	if (mPDFPlayer) return mPDFPlayer->getPDFInterface();
	if (mStreamPlayer) return mStreamPlayer->getVideoInterface();
	if (mWebPlayer) return mWebPlayer->getWebInterface();
	return nullptr;
}

void MediaPlayer::setErrorCallback(std::function<void(const std::string& msg)> func) { mErrorCallback = func; }

void MediaPlayer::setStatusCallback(std::function<void(const bool isGood)> func) { mStatusCallback = func; }

void MediaPlayer::setInitializedCallback(std::function<void()> func) { mInitializedCallback = func; }

void MediaPlayer::handleStandardClick(const ci::vec3& globalPos) {
	if (mWebPlayer) mWebPlayer->sendClick(globalPos);
	if (mPDFPlayer) mPDFPlayer->nextPage();
	if (mVideoPlayer) mVideoPlayer->togglePlayPause();
	if (mPanoramicPlayer) mPanoramicPlayer->togglePlayPause();
}

void MediaPlayer::enableStandardClick() {
	setTapCallback([this](ds::ui::Sprite* bs, const ci::vec3& pos) { handleStandardClick(pos); });
}

}  // namespace ui
}  // namespace ds
