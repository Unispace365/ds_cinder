#include "ds/ui/sprite/gst_video.h"

#include <cinder/Camera.h>
#include <ds/app/app.h>
#include <ds/app/blob_reader.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine_io_defs.h>
#include <ds/data/data_buffer.h>
#include <ds/data/resource_list.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>
#include <ds/math/math_func.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "gstreamer/_2RealGStreamerWrapper.h"
#include "gstreamer/video_meta_cache.h"

using namespace ci;
using namespace _2RealGStreamerWrapper;

namespace {
// Statically initialize in the app thread.
class Init {
public:
	Init() {
		// Set the main gstreamer path. We're assuming it's in the standard location;
		// if that ever changes, then load up a settings file here and get the path from that.
		ds::Environment::addToFrontEnvironmentVariable("PATH", "C:\\gstreamer\\1.0\\x86\\bin");

		// Add a startup object to set the plugin path. This is how we'd prefer to
		// do both path setups, but we had to delay-load some DLLs for gstreamer,
		// so we're being extracautious about the path variable.
		ds::App::AddStartup([](ds::Engine& e) {
			std::string		plugin_path("GST_PLUGIN_PATH=C:\\gstreamer\\1.0\\x86\\lib\\gstreamer-1.0");
			_putenv(plugin_path.c_str());

			e.installSprite([](ds::BlobRegistry& r){ds::ui::GstVideo::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::GstVideo::installAsClient(r);});
		});
	}
};
Init						INIT;

char						BLOB_TYPE			= 0;
const ds::ui::DirtyState&	FILENAME_DIRTY		= ds::ui::INTERNAL_A_DIRTY;
const ds::ui::DirtyState&	VOLUME_DIRTY		= ds::ui::INTERNAL_B_DIRTY;
const ds::ui::DirtyState&	LOOPING_DIRTY		= ds::ui::INTERNAL_C_DIRTY;
const ds::ui::DirtyState&	CMD_DIRTY			= ds::ui::INTERNAL_D_DIRTY;
const ds::ui::DirtyState&	VIDEO_FLAGS_DIRTY	= ds::ui::INTERNAL_E_DIRTY;
const char					FILENAME_ATT		= 80;
const char					VOLUME_ATT			= 81;
const char					LOOPING_ATT			= 82;
const char					CMD_ATT				= 83;
const char					VIDEO_FLAGS_ATT		= 84;

const uint32_t				CHECKBOUNDSHACK_F	= (1<<0);

// Status atts
const char					STATUS_POSITION_ATT	= 100;
}

static _2RealGStreamerWrapper::GStreamerWrapper* new_movie() {
	_2RealGStreamerWrapper::GStreamerWrapper*	ans = new _2RealGStreamerWrapper::GStreamerWrapper();
	if (!ans) throw std::runtime_error("GStreamer Video can't create mMovie");
	return ans;
}

namespace {
ds::ui::VideoMetaCache		CACHE("gstreamer");
}

namespace ds {
namespace ui {

/**
 * \class ds::ui::sprite::Video static
 */
void GstVideo::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void GstVideo::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<GstVideo>(r);});
}

GstVideo& GstVideo::makeVideo(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::GstVideo>([&e]()->ds::ui::GstVideo*{ return new ds::ui::GstVideo(e); }, parent);
}

/**
 * \class ds::ui::sprite::Video static
 */
GstVideo::GstVideo(SpriteEngine& engine)
		: inherited(engine)
		, mMoviePtr(new_movie())
		, mMovie(*mMoviePtr)
		, mFilenameChanged(false)
		, mLooping(false)
		, mMuted(false)
		, mInternalMuted(true)
		, mVolume(1.0f)
		, mStatusDirty(false)
		, mStatusFn(nullptr)
		, mIsTransparent(true)
		, mCmd(kCmdStop)
		, mPlaySingleFrame(false)
		, mPlaySingleFrameFn(nullptr)
		, mServerModeHack(false)
		, mReportedCurrentPosition(0.0)
		, mVideoFlags(0) {
	mBlobType = BLOB_TYPE;

	setUseShaderTextuer(true);
	setTransparent(false);
	setStatus(Status::STATUS_STOPPED);
}

GstVideo::~GstVideo() {
	mMovie.stop();
	mMovie.close();
	delete mMoviePtr;
}

void GstVideo::updateClient(const UpdateParams &up) {
	inherited::updateClient(up);

	// Total check bounds hack, stop if out of bounds
	if (checkBounds() && !inBounds()) {
		mMovie.stop();
		return;
	}

	if (mFilenameChanged) {
		mFilenameChanged = false;
		const std::string		fn(ds::Environment::expand(mFilename));
		doLoadVideo(fn);
	}

	if (mStatusDirty) {
		mStatusDirty = false;
		if (mStatusFn) mStatusFn(mStatus);
	}
			
	mMovie.update();
}

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
			
	mMovie.update();
}

void GstVideo::drawLocalClient() {
	if (!mFbo) return;

	if (mMovie.getState() == STOPPED) setStatus(Status::STATUS_STOPPED);
	else if (mMovie.getState() == PLAYING) setStatus(Status::STATUS_PLAYING);
	else setStatus(Status::STATUS_PAUSED);

	if (!inBounds()) {
		if (!mInternalMuted) {
			mMovie.setVolume(0.0f);
			mInternalMuted = true;
		}
		return;
	}

	if (mInternalMuted) {
		mInternalMuted = false;
		setMovieVolume();
	}
	if(mMovie.hasVideo() && mMovie.isNewVideoFrame()){
		unsigned char* pImg = mMovie.getVideo();
		if(pImg != nullptr){		
			int vidWidth( mMovie.getWidth()), vidHeight(mMovie.getHeight());
			if(mIsTransparent){
				mFrameTexture = ci::gl::Texture(pImg, GL_RGBA, vidWidth, vidHeight);
			} else {
				mFrameTexture = ci::gl::Texture(pImg, GL_RGB, vidWidth, vidHeight);
			}
			// 	DS_LOG_INFO("New video frame, texture id: " <<mFrameTexture.getId());
			DS_REPORT_GL_ERRORS();
		}
		if(mPlaySingleFrame){
			stop();
			mPlaySingleFrame = false;
			if (mPlaySingleFrameFn) mPlaySingleFrameFn(*this);
			mPlaySingleFrameFn = nullptr;
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

			if(mIsTransparent){
				ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
			} else {
				ci::gl::clear(ci::Color(0.0f, 0.0f, 0.0f));
			}
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
	DS_LOG_INFO("GstVideo::loadVideo() on " << _filename);
	if (_filename == mFilename) return *this;

	const std::string			filename(ds::Environment::expand(_filename));
	if (filename.empty()) {
		DS_LOG_WARNING("GstVideo::loadVideo recieved a blank filename. Cancelling load.");
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
	markAsDirty(LOOPING_DIRTY);
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
	markAsDirty(VOLUME_DIRTY);
}

float GstVideo::getVolume() const {
	return mVolume;
}

void GstVideo::play() {
	DS_LOG_INFO("GstVideo::play()");

	mMovie.play();
	setCmd(kCmdPlay);
}

void GstVideo::stop() {
	DS_LOG_INFO("GstVideo::stop()");

	mMovie.stop();
	setCmd(kCmdStop);
}

void GstVideo::pause() {
	DS_LOG_INFO("GstVideo::pause()");
	mMovie.pause();
	setCmd(kCmdPause);
}

bool GstVideo::getIsPlaying() const {
	if (mServerModeHack) {
		return mCmd == kCmdPlay;
	}

	if(mMovie.getState() == PLAYING){
		return true;
	} else {
		return false;
	}
}

double GstVideo::getDuration() const {
	return mMovie.getDurationInMs() / 1000.0;
}

double GstVideo::getCurrentTime() const {
	return mMovie.getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t) {
	mMovie.setTimePositionInMs(t * 1000.0);
}

double GstVideo::getCurrentPosition() const {
	if (mServerModeHack) {
		return mReportedCurrentPosition;
	}
	return mMovie.getPosition();
}

void GstVideo::seekPosition(const double t) {
	mMovie.setPosition(t);
}

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE || mEngine.getMode() == mEngine.STANDALONE_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
}

void GstVideo::writeAttributesTo(ds::DataBuffer &buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(FILENAME_DIRTY)) {
		buf.add(FILENAME_ATT);
		buf.add(mFilename);
	}
	if (mDirty.has(VOLUME_DIRTY)) {
		buf.add(VOLUME_ATT);
		buf.add(mVolume);
	}
	if (mDirty.has(LOOPING_DIRTY)) {
		buf.add(LOOPING_ATT);
		buf.add(mLooping);
	}
	if (mDirty.has(CMD_DIRTY)) {
		buf.add(CMD_ATT);
		buf.add(mCmd);
	}
	if (mDirty.has(VIDEO_FLAGS_DIRTY)) {
		buf.add(VIDEO_FLAGS_ATT);
		buf.add(mVideoFlags);
	}
}

void GstVideo::writeClientAttributesTo(ds::DataBuffer &buf) const {
	ScopedClientAtts		scope(buf, getId());
	buf.add(static_cast<float>(getCurrentPosition()));
}

void GstVideo::readAttributeFrom(const char attributeId, ds::DataBuffer &buf) {
	if (attributeId == FILENAME_ATT) {
		onSetFilename(buf.read<std::string>());
		const std::string	fn(ds::Environment::expand(mFilename));
		doLoadVideoMeta(fn);
	} else if (attributeId == VOLUME_ATT) {
		setVolume(buf.read<float>());
	} else if (attributeId == LOOPING_ATT) {
		setLooping(buf.read<bool>());
	} else if (attributeId == CMD_ATT) {
		setCmd(buf.read<Cmd>());
		if (mCmd == kCmdPlay) play();
		else if (mCmd == kCmdPause) pause();
		else if (mCmd == kCmdStop) stop();
	} else if (attributeId == VIDEO_FLAGS_ATT) {
		mVideoFlags = buf.read<uint32_t>();
		setCheckBounds((mVideoFlags&CHECKBOUNDSHACK_F) != 0);
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

void GstVideo::readClientFrom(ds::DataBuffer &buf) {
	mReportedCurrentPosition = buf.read<float>();
}

void GstVideo::doLoadVideoMeta(const std::string &filename) {
	DS_LOG_INFO("GstVideo::doLoadVideoMeta() on " << filename);
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
	DS_LOG_INFO("GstVideo::doLoadVideo() on " << filename);
	if (mServerModeHack) return;
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

		DS_LOG_INFO("GstVideo::doLoadVideo() movieOpen");
		mMovie.open(filename, generateVideoBuffer, false, mIsTransparent, videoWidth, videoHeight);

		setMovieLooping();
		//mMovie.play();
		setMovieVolume();
		if (type == VideoMetaCache::AUDIO_TYPE) {
			// Otherwise I am permanently muted
			mInternalMuted = false;
		} else {
			mInternalMuted = true;
		}
		mMovie.setVideoCompleteCallback([this](GStreamerWrapper* video){ handleVideoComplete(video);});
		setStatus(Status::STATUS_PLAYING);

	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::doLoadVideo() ex=" << ex.what() << std::endl);
		return;
	}

	if (mMovie.getWidth() < 1.0f || mMovie.getHeight() < 1.0f) {
		if (type != VideoMetaCache::AUDIO_TYPE) {
			DS_LOG_WARNING("GstVideo::doLoadVideo() Video is too small to be used or didn't load correctly! " << filename << " " << getWidth() << " " << getHeight());
		}
		return;
	}
	mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
}

void GstVideo::onSetFilename(const std::string &fn) {
	mFilename = fn;
	mFilenameChanged = true;
	markAsDirty(FILENAME_DIRTY);
}

void GstVideo::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusDirty = true;
}

void GstVideo::setMovieVolume() {
	if (mMuted || mInternalMuted) {
		mMovie.setVolume(0.0f);
	} else {
		mMovie.setVolume(mVolume);
	}
}

void GstVideo::setMovieLooping() {
	if (mLooping) {
		mMovie.setLoopMode(LOOP);
	} else {
		mMovie.setLoopMode(NO_LOOP);
	}
}

void GstVideo::unloadVideo(const bool clearFrame) {
	mMovie.stop();
	mMovie.close();
	if (clearFrame) {
		mFrameTexture.reset();
	}
}

// set this before loading a video
void GstVideo::setAlphaMode( bool isTransparent ) {
	mIsTransparent = isTransparent;
}

void GstVideo::setVideoCompleteCallback( const std::function<void(GstVideo* video)> &func ) {
	mVideoCompleteCallback = func;
}

void GstVideo::handleVideoComplete(GStreamerWrapper* wrapper) {
	if (mVideoCompleteCallback) {
		mVideoCompleteCallback(this);
	}
}

void GstVideo::setAutoStart( const bool doAutoStart ) {
	mMovie.setStartPlaying(doAutoStart);
}

void GstVideo::playAFrame(const std::function<void(GstVideo&)>& fn) {
	mPlaySingleFrame = true;
	mPlaySingleFrameFn = fn;
	if(!getIsPlaying()) {
		play();
	}
}

bool GstVideo::isPlayingAFrame() const {
	return mPlaySingleFrame;
}

void GstVideo::stopAfterNextLoop() {
	mMovie.stopOnLoopComplete();
}

void GstVideo::setServerModeHack(const bool b) {
	mServerModeHack = b;
}

void GstVideo::setCheckBoundsHack(const bool b) {
	setVideoFlag(CHECKBOUNDSHACK_F, b);
	setCheckBounds((mVideoFlags&CHECKBOUNDSHACK_F) != 0);
}

void GstVideo::setCmd(const Cmd c) {
	mCmd = c;
	markAsDirty(CMD_DIRTY);
}

void GstVideo::setVideoFlag(const uint32_t f, const bool on) {
	const uint32_t		old(mVideoFlags);
	if (on) mVideoFlags |= f;
	else mVideoFlags &= ~f;

	if (old != mVideoFlags) {
		markAsDirty(VIDEO_FLAGS_DIRTY);
	}
}

} // namespace ui
} // namespace ds
