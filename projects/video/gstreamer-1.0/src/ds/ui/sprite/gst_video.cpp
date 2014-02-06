#include "ds/ui/sprite/gst_video.h"

#include <cinder/Camera.h>
#include <ds/app/app.h>
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
		ds::App::AddStartup([](ds::Engine& e) {
#if 0
			std::string		new_path("PATH=");
			const char*		path_env = getenv("PATH");
			if (path_env) {
				new_path += path_env;
			}
			if (new_path.length() > 0) {
				if (!(new_path.back() == '=' || new_path.back() == ';')) {
					new_path += ";";
				}
			}
			new_path += "C:\\gstreamer\\1.0\\x86\\bin;C:\\gstreamer\\1.0\\x86\\lib\\gstreamer-1.0";
			_putenv(new_path.c_str());
#endif

			std::string		plugin_path("GST_PLUGIN_PATH=C:\\gstreamer\\1.0\\x86\\lib\\gstreamer-1.0");
			_putenv(plugin_path.c_str());
		});

	}
};
Init				INIT;
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

GstVideo::GstVideo( SpriteEngine& engine )
		: inherited(engine)
		, mMoviePtr(new_movie())
		, mMovie(*mMoviePtr)
		, mLooping(false)
		, mMuted(false)
		, mInternalMuted(true)
		, mVolume(1.0f)
		, mStatusDirty(false)
		, mStatusFn(nullptr)
		, mIsTransparent(true)
		, mPlaySingleFrame(false)
		, mPlaySingleFrameFn(nullptr) {
	setUseShaderTextuer(true);
	setTransparent(false);
	setStatus(Status::STATUS_STOPPED);
}

GstVideo::~GstVideo() {
	mMovie.stop();
	mMovie.close();
	delete mMoviePtr;
}

void GstVideo::updateServer(const UpdateParams& up) {
	inherited::updateServer(up);

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

GstVideo& GstVideo::loadVideo( const std::string &filename) {
	if(filename.empty()){
		DS_LOG_WARNING("GstVideo::loadVideo recieved a blank filename. Cancelling load.");
		return *this;
	}

	try {
		int videoWidth = static_cast<int>(getWidth());
		int videoHeight = static_cast<int>(getHeight());
		if(videoWidth < 1 || videoHeight < 1){
			CACHE.getSize(filename, videoWidth, videoHeight);
		}

		mMovie.open( filename, true, false, mIsTransparent, videoWidth, videoHeight );

		if(mLooping){
			mMovie.setLoopMode(LOOP);
		} else {
			mMovie.setLoopMode(NO_LOOP);
		}
		//mMovie.play();
		setMovieVolume();
		mInternalMuted = true;
		mMovie.setVideoCompleteCallback([this](GStreamerWrapper* video){ handleVideoComplete(video);});
		setStatus(Status::STATUS_PLAYING);
	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::loadVideo() ex=" << ex.what() << std::endl);
		return *this;
	}

	if(mMovie.getWidth() < 1.0f || mMovie.getHeight() < 1.0f){
		DS_LOG_WARNING("GstVideo::loadVideo() Video is too small to be used or didn't load correctly! " << filename << " " << getWidth() << " " << getHeight());
		return *this;
	}

	Sprite::setSizeAll(static_cast<float>(mMovie.getWidth()), static_cast<float>(mMovie.getHeight()), mDepth);

	if (getWidth() > 0 &&  getHeight() > 0) {
		setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
	}
	mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
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

	mFbo = ci::gl::Fbo(static_cast<int>(getWidth()), static_cast<int>(getHeight()), true);
	return *this;
}

void GstVideo::setLooping(const bool on) {
	mLooping = on;
	if(mLooping){
		mMovie.setLoopMode(LOOP);
	} else {
		mMovie.setLoopMode(NO_LOOP);
	}
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

void GstVideo::setVolume(const float volume ) {
	mVolume = volume;
	setMovieVolume();
}

float GstVideo::getVolume() const {
	return mVolume;
}

void GstVideo::play() {
	mMovie.play();
}

void GstVideo::stop() {
	mMovie.stop();
}

void GstVideo::pause() {
	mMovie.pause();
}

bool GstVideo::getIsPlaying() const {
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
	return mMovie.getPosition();
}

void GstVideo::seekPosition(const double t) {
	mMovie.setPosition(t);
}

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn) {
	DS_ASSERT_MSG(mEngine.getMode() == mEngine.CLIENTSERVER_MODE, "Currently only works in ClientServer mode, fill in the UDP callbacks if you want to use this otherwise");
	mStatusFn = fn;
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

void GstVideo::unloadVideo() {
	mMovie.stop();
	mMovie.close();
}

// set this before loading a video
void GstVideo::setAlphaMode( bool isTransparent ) {
	mIsTransparent = isTransparent;
}

void GstVideo::setVideoCompleteCallback( const std::function<void(GstVideo* video)> &func ) {
	mVideoCompleteCallback = func;
}

void GstVideo::handleVideoComplete(GStreamerWrapper* wrapper) {
	if(mVideoCompleteCallback){
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

} // namespace ui
} // namespace ds
