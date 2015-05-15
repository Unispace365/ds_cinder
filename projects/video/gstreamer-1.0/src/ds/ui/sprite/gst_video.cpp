#include "ds/ui/sprite/gst_video.h"

#include <cinder/Camera.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_reader.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine_io_defs.h>
#include <ds/data/data_buffer.h>
#include <ds/data/resource_list.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite_engine.h>

#include <sstream>

#include <Poco/Path.h>

#include "gstreamer/_2RealGStreamerWrapper.h"
#include "gstreamer/gstreamer_env_check.h"
#include "gstreamer/video_meta_cache.h"

using namespace ci;
using namespace _2RealGStreamerWrapper;

namespace {
static ds::gstreamer::EnvCheck  ENV_CHECK;
ds::ui::VideoMetaCache          CACHE("gstreamer");
const ds::BitMask               GSTREAMER_LOG = ds::Logger::newModule("gstreamer");
} //!anonymous namespace

namespace ds {
namespace ui {

/*!
 * \class Impl
 * \namespace ds::ui
 * \brief Pimpl provider of GstVideo.
 * \note moved here to hide the forward declaration of GStreamerWrapper.
 */
class Impl {
public:
	Impl(GstVideo& holder)
		: mHolder(holder)
		, mMoviePtr(std::make_unique<GStreamerWrapper>())
	{}

	void handleVideoComplete(GStreamerWrapper*) {
		mHolder.triggerVideoCompleteCallback();
	}

	GStreamerWrapper& getMovieRef() {
		return *mMoviePtr;
	}

	~Impl() {
		mMoviePtr->stop();
		mMoviePtr->close();
	}

private:
	GstVideo& mHolder;
	std::unique_ptr<GStreamerWrapper> mMoviePtr;
};

GstVideo& GstVideo::makeVideo(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::GstVideo>([&e]()->ds::ui::GstVideo*{ return new ds::ui::GstVideo(e); }, parent);
}

/**
 * \class ds::ui::sprite::Video static
 */
GstVideo::GstVideo(SpriteEngine& engine)
	: inherited(engine)
	, mGstreamerWrapper(std::make_shared<Impl>(*this))
	, mFilenameChanged(false)
	, mLooping(false)
	, mMuted(false)
	, mInternalMuted(true)
	, mVolume(1.0f)
	, mStatusDirty(false)
	, mStatusFn(nullptr)
	, mShouldPlay(false)
	, mAutoStart(false)
{
	setUseShaderTextuer(true);
	setStatus(Status::STATUS_STOPPED);
}

GstVideo::~GstVideo() {}

void GstVideo::updateServer(const UpdateParams &up) {
	inherited::updateServer(up);

	if (mFilenameChanged) {
		mFilenameChanged = false;
		const std::string		fn(ds::Environment::expand(mFilename));
		doLoadVideo(fn);
	}

	if (mStatusDirty) {
		mStatusDirty = false;
		if (mStatusFn) mStatusFn(mStatus);
	}
			
	mGstreamerWrapper->getMovieRef().update();

	if (mGstreamerWrapper->getMovieRef().hasVideo() && mShouldPlay) {
		play();
		mShouldPlay = false;
	}
}

void GstVideo::drawLocalClient() {
	if (!mFbo) return;

	if (mGstreamerWrapper->getMovieRef().getState() == STOPPED) setStatus(Status::STATUS_STOPPED);
	else if (mGstreamerWrapper->getMovieRef().getState() == PLAYING) setStatus(Status::STATUS_PLAYING);
	else setStatus(Status::STATUS_PAUSED);

	if (!inBounds()) {
		if (!mInternalMuted) {
			mGstreamerWrapper->getMovieRef().setVolume(0.0f);
			mInternalMuted = true;
		}
		return;
	}

	if (mInternalMuted) {
		mInternalMuted = false;
		setMovieVolume();
	}
	if (mGstreamerWrapper->getMovieRef().hasVideo() && mGstreamerWrapper->getMovieRef().isNewVideoFrame()){
		unsigned char* pImg = mGstreamerWrapper->getMovieRef().getVideo();
		if(pImg != nullptr){		
			int vidWidth(mGstreamerWrapper->getMovieRef().getWidth()), vidHeight(mGstreamerWrapper->getMovieRef().getHeight());
		    mFrameTexture = ci::gl::Texture(pImg, GL_RGBA, vidWidth, vidHeight);
			DS_REPORT_GL_ERRORS();
		}
	}

	if ( mFrameTexture ) {
		{
			ci::gl::pushMatrices();
			mSpriteShader.getShader().unbind();
			ci::gl::setViewport(mFrameTexture.getBounds());
			ci::CameraOrtho camera;
			camera.setOrtho(float(mFrameTexture.getBounds().getX1()), float(mFrameTexture.getBounds().getX2()), float(mFrameTexture.getBounds().getY2()), float(mFrameTexture.getBounds().getY1()), -1.0f, 1.0f);
			ci::gl::setMatrices(camera);
			// bind the framebuffer - now everything we draw will go there
			mFbo.bindFramebuffer();

			glPushAttrib( GL_TRANSFORM_BIT | GL_ENABLE_BIT );
			for (int i = 0; i < 4; ++i) {
				glDisable( GL_CLIP_PLANE0 + i );
			}

			ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
			DS_REPORT_GL_ERRORS();

			ci::gl::draw(mFrameTexture);
			DS_REPORT_GL_ERRORS();

			glPopAttrib();

			mFbo.unbindFramebuffer();
			mSpriteShader.getShader().bind();
			ci::gl::popMatrices();
		}

		Rectf screenRect = mEngine.getScreenRect();
		ci::gl::setViewport(Area((int)screenRect.getX1(), (int)screenRect.getY2(), (int)screenRect.getX2(), (int)screenRect.getY1()));

		if (getPerspective()) {
			Rectf area(0.0f, 0.0f, getWidth(), getHeight());
			ci::gl::draw( mFbo.getTexture(0), area );
		} else {
			Rectf area(0.0f, getHeight(), getWidth(), 0.0f);
			ci::gl::draw( mFbo.getTexture(0), area );
		}

		DS_REPORT_GL_ERRORS();
	}
}

void GstVideo::setSize( float width, float height ) {
	setScale( width / getWidth(), height / getHeight() );
}

GstVideo& GstVideo::loadVideo(const std::string &_filename) {
	DS_LOG_INFO_M("GstVideo::loadVideo() on " << _filename, GSTREAMER_LOG);
	if (_filename == mFilename) return *this;

	const std::string			filename(ds::Environment::expand(_filename));
	if (filename.empty()) {
		DS_LOG_WARNING_M("GstVideo::loadVideo recieved a blank filename. Cancelling load.", GSTREAMER_LOG);
		return *this;
	}

	doLoadVideoMeta(filename);
	onSetFilename(_filename);
	return *this;
}

GstVideo &GstVideo::setResourceId( const ds::Resource::Id &resourceId ) {
	try {
		ds::Resource            res;
		if (mEngine.getResources().get(resourceId, res)) {
			Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
			std::string filename = res.getAbsoluteFilePath();
			loadVideo(filename);
		}
	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::loadVideo() ex=" << ex.what() << std::endl);
		return *this;
	}
	return *this;
}

void GstVideo::setLooping(const bool on) {
	mLooping = on;
	setMovieLooping();
}

bool GstVideo::getIsLooping() const {
	return mLooping;
}
		
void GstVideo::setMute( const bool doMute ) {
	mMuted = doMute;
	setMovieVolume();
}

bool GstVideo::getIsMuted() const {
	return mMuted;
}

void GstVideo::setVolume(const float volume) {
	if (mVolume == volume) return;

	mVolume = volume;
	setMovieVolume();
}

float GstVideo::getVolume() const {
	return mVolume;
}

void GstVideo::play() {
	DS_LOG_INFO_M("GstVideo::play() " << mFilename, GSTREAMER_LOG);

	mGstreamerWrapper->getMovieRef().play();

	//If movie not yet loaded, remember to play it later once it has
	if (!mGstreamerWrapper->getMovieRef().hasVideo())  {
		mShouldPlay = true;
	}
}

void GstVideo::stop() {
	DS_LOG_INFO_M("GstVideo::stop() " << mFilename, GSTREAMER_LOG);

	mShouldPlay = false;

	mGstreamerWrapper->getMovieRef().stop();
}

void GstVideo::pause() {
	DS_LOG_INFO_M("GstVideo::pause() " << mFilename, GSTREAMER_LOG);

	mShouldPlay = false;

	mGstreamerWrapper->getMovieRef().pause();
}

bool GstVideo::getIsPlaying() const {
	if (mGstreamerWrapper->getMovieRef().getState() == PLAYING){
		return true;
	} else {
		return false;
	}
}

double GstVideo::getDuration() const {
	return mGstreamerWrapper->getMovieRef().getDurationInMs() / 1000.0;
}

double GstVideo::getCurrentTime() const {
	return mGstreamerWrapper->getMovieRef().getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t) {
	mGstreamerWrapper->getMovieRef().setTimePositionInMs(t * 1000.0);
}

double GstVideo::getCurrentPosition() const {
	return mGstreamerWrapper->getMovieRef().getPosition();
}

void GstVideo::seekPosition(const double t) {
	mGstreamerWrapper->getMovieRef().setPosition(t);
}

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE || mEngine.getMode() == mEngine.STANDALONE_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void GstVideo::doLoadVideoMeta(const std::string &filename) {
	DS_LOG_INFO_M("GstVideo::doLoadVideoMeta() on " << filename, GSTREAMER_LOG);
	if (filename.empty()) return;
	
	try {
		VideoMetaCache::Type	type(VideoMetaCache::ERROR_TYPE);
		int						videoWidth = static_cast<int>(getWidth());
		int						videoHeight = static_cast<int>(getHeight());
		double					videoDuration(0.0f);
		CACHE.getValues(filename, type, videoWidth, videoHeight, videoDuration);
		bool					generateVideoBuffer = true;
		if (type == VideoMetaCache::AUDIO_TYPE) {
			generateVideoBuffer = false;
			mInternalMuted = false;
		}

		Sprite::setSizeAll(static_cast<float>(videoWidth), static_cast<float>(videoHeight), mDepth);

		if (getWidth() > 0 &&  getHeight() > 0) {
			setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
		}
	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::doLoadVideoMeta() ex=" << ex.what() << std::endl);
		return;
	}
}

void GstVideo::doLoadVideo(const std::string &filename) {
	DS_LOG_INFO_M("GstVideo::doLoadVideo() on " << filename, GSTREAMER_LOG);
	if (filename.empty()) return;
	
	VideoMetaCache::Type		type(VideoMetaCache::ERROR_TYPE);
	try {
		int						videoWidth = static_cast<int>(getWidth());
		int						videoHeight = static_cast<int>(getHeight());
		double					videoDuration(0.0f);
		CACHE.getValues(filename, type, videoWidth, videoHeight, videoDuration);
		bool					generateVideoBuffer = true;
		if (type == VideoMetaCache::AUDIO_TYPE) {
			generateVideoBuffer = false;
			mInternalMuted = false;
		}

		Sprite::setSizeAll(static_cast<float>(videoWidth), static_cast<float>(videoHeight), mDepth);

		if (getWidth() > 0 &&  getHeight() > 0) {
			setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
		}

		DS_LOG_INFO_M("GstVideo::doLoadVideo() movieOpen", GSTREAMER_LOG);
		mGstreamerWrapper->getMovieRef().open(filename, generateVideoBuffer, false, true, videoWidth, videoHeight);

		setMovieLooping();
		setMovieVolume();
		if (type == VideoMetaCache::AUDIO_TYPE) {
			// Otherwise I am permanently muted
			mInternalMuted = false;
		} else {
			mInternalMuted = true;
		}
		mGstreamerWrapper->getMovieRef().setVideoCompleteCallback([this](GStreamerWrapper* video){ mGstreamerWrapper->handleVideoComplete(video); });
		setStatus(Status::STATUS_PLAYING);

	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::doLoadVideo() ex=" << ex.what() << std::endl);
		return;
	}

	if (mGstreamerWrapper->getMovieRef().getWidth() < 1.0f || mGstreamerWrapper->getMovieRef().getHeight() < 1.0f) {
		if (type != VideoMetaCache::AUDIO_TYPE) {
			DS_LOG_WARNING_M("GstVideo::doLoadVideo() Video is too small to be used or didn't load correctly! " << filename << " " << getWidth() << " " << getHeight(), GSTREAMER_LOG);
		}
		return;
	}
	mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
}

void GstVideo::onSetFilename(const std::string &fn) {
	mFilename = fn;
	mFilenameChanged = true;
}

void GstVideo::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusDirty = true;
}

void GstVideo::setMovieVolume() {
	if (mMuted || mInternalMuted) {
		mGstreamerWrapper->getMovieRef().setVolume(0.0f);
	} else {
		mGstreamerWrapper->getMovieRef().setVolume(mVolume);
	}
}

void GstVideo::setMovieLooping() {
	if (mLooping) {
		mGstreamerWrapper->getMovieRef().setLoopMode(LOOP);
	} else {
		mGstreamerWrapper->getMovieRef().setLoopMode(NO_LOOP);
	}
}

void GstVideo::unloadVideo(const bool clearFrame) {
	mGstreamerWrapper->getMovieRef().stop();
	mGstreamerWrapper->getMovieRef().close();
	mFilename.clear();
	if (clearFrame) {
		mFrameTexture.reset();
	}
}

void GstVideo::setVideoCompleteCallback( const std::function<void(GstVideo* video)> &func ) {
	mVideoCompleteFn = func;
}

void GstVideo::triggerVideoCompleteCallback()
{
	if (mVideoCompleteFn) {
		mVideoCompleteFn(this);
	}
}

void GstVideo::setAutoStart( const bool doAutoStart ) {
	// do not check for mAutoStart == doAutoStart. There is no
	// correlation between them. mAutoStart is a cache value. Might be out of sync.
	mAutoStart = doAutoStart;
	mGstreamerWrapper->getMovieRef().setStartPlaying(mAutoStart);
}

bool GstVideo::getAutoStart() const
{
	return mAutoStart;
}

void GstVideo::stopAfterNextLoop() {
	mGstreamerWrapper->getMovieRef().stopOnLoopComplete();
}

} //!namespace ui
} //!namespace ds