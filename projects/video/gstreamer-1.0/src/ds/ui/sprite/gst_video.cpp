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
#include "gstreamer/video_meta_cache.h"

using namespace ci;
using namespace _2RealGStreamerWrapper;

namespace {
// Statically initialize in the app thread.
class Init {
public:
	Init() {

		// Load the environment variable and check if it exists.
		// If it does not exist, use the default legacy install location
		char * cindEnv = getenv("DS_CINDER_GSTREAMER_1-0");
		std::string cinderGstreamerPath = "";
		if (cindEnv){
			cinderGstreamerPath = cindEnv;
		} else {
			cinderGstreamerPath = "c:/gstreamer/1.0/x86";
		}

		// Add the bin to the env path
		std::stringstream pathy;
		pathy << cinderGstreamerPath << "\\bin";

		// Set the main gstreamer path from the environment variable or default
		ds::Environment::addToFrontEnvironmentVariable("PATH", Poco::Path::expand(pathy.str()));

		// Add a startup object to set the plugin path. This is how we'd prefer to
		// do both path setups, but we had to delay-load some DLLs for gstreamer,
		// so we're being extracautious about the path variable.
		ds::App::AddStartup([cinderGstreamerPath](ds::Engine& e) {

			// Use the previously determined base path and add the dll location
			std::stringstream pathToExpand;
			pathToExpand << cinderGstreamerPath << "\\lib\\gstreamer-1.0";
			std::stringstream ss;
			ss << "GST_PLUGIN_PATH=" << Poco::Path::expand(pathToExpand.str());
			std::string		plugin_path(ss.str());
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

ds::ui::VideoMetaCache		CACHE("gstreamer");

const ds::BitMask			GSTREAMER_LOG = ds::Logger::newModule("gstreamer");
}

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
		, mPimpl(std::make_shared<Impl>(*this))
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
		, mVideoFlags(0)
		, mDoPlay(false)
		, mAutoStart(false)
{
	mBlobType = BLOB_TYPE;

	setUseShaderTextuer(true);
	setTransparent(false);
	setStatus(Status::STATUS_STOPPED);
}

GstVideo::~GstVideo() {}

void GstVideo::updateClient(const UpdateParams &up) {
	inherited::updateClient(up);

	// Total check bounds hack, stop if out of bounds
	if (checkBounds() && !inBounds()) {
		mPimpl->getMovieRef().stop();
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
			
	mPimpl->getMovieRef().update();
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
			
	mPimpl->getMovieRef().update();

	if (mPimpl->getMovieRef().hasVideo() && mDoPlay) {
		play();
		mDoPlay = false;
	}
}

void GstVideo::drawLocalClient() {
	if (!mFbo) return;

	if (mPimpl->getMovieRef().getState() == STOPPED) setStatus(Status::STATUS_STOPPED);
	else if (mPimpl->getMovieRef().getState() == PLAYING) setStatus(Status::STATUS_PLAYING);
	else setStatus(Status::STATUS_PAUSED);

	if (!inBounds()) {
		if (!mInternalMuted) {
			mPimpl->getMovieRef().setVolume(0.0f);
			mInternalMuted = true;
		}
		return;
	}

	if (mInternalMuted) {
		mInternalMuted = false;
		setMovieVolume();
	}
	if (mPimpl->getMovieRef().hasVideo() && mPimpl->getMovieRef().isNewVideoFrame()){
		unsigned char* pImg = mPimpl->getMovieRef().getVideo();
		if(pImg != nullptr){		
			int vidWidth(mPimpl->getMovieRef().getWidth()), vidHeight(mPimpl->getMovieRef().getHeight());
			if(mIsTransparent){
				mFrameTexture = ci::gl::Texture(pImg, GL_RGBA, vidWidth, vidHeight);
			} else {
				mFrameTexture = ci::gl::Texture(pImg, GL_RGB, vidWidth, vidHeight);
			}
			// 	DS_LOG_INFO_M("New video frame, texture id: " <<mFrameTexture.getId(), GSTREAMER_LOG);
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
	DS_LOG_INFO_M("GstVideo::play() " << mFilename, GSTREAMER_LOG);

	mPimpl->getMovieRef().play();
	setCmd(kCmdPlay);

	//If movie not yet loaded, remember to play it later once it has
	if (!mPimpl->getMovieRef().hasVideo())  {
		mDoPlay = true;
	}
}

void GstVideo::stop() {
	DS_LOG_INFO_M("GstVideo::stop() " << mFilename, GSTREAMER_LOG);

	mDoPlay = false;

	mPimpl->getMovieRef().stop();
	setCmd(kCmdStop);
}

void GstVideo::pause() {
	DS_LOG_INFO_M("GstVideo::pause() " << mFilename, GSTREAMER_LOG);

	mDoPlay = false;

	mPimpl->getMovieRef().pause();
	setCmd(kCmdPause);
}

bool GstVideo::getIsPlaying() const {
	if (mServerModeHack) {
		return mCmd == kCmdPlay;
	}

	if (mPimpl->getMovieRef().getState() == PLAYING){
		return true;
	} else {
		return false;
	}
}

double GstVideo::getDuration() const {
	return mPimpl->getMovieRef().getDurationInMs() / 1000.0;
}

double GstVideo::getCurrentTime() const {
	return mPimpl->getMovieRef().getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t) {
	mPimpl->getMovieRef().setTimePositionInMs(t * 1000.0);
}

double GstVideo::getCurrentPosition() const {
	if (mServerModeHack) {
		return mReportedCurrentPosition;
	}
	return mPimpl->getMovieRef().getPosition();
}

void GstVideo::seekPosition(const double t) {
	mPimpl->getMovieRef().setPosition(t);
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
	inherited::writeClientAttributesTo(buf);

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

void GstVideo::readClientAttributeFrom(const char attributeId, ds::DataBuffer& buf)
{
	// Server hack mode was coded for GMI and it does not do anything anymore (SL.)
	/*if (attributeId == CURR_POS_ATT) {
		mReportedCurrentPosition = buf.read<float>();
	} else */{
		inherited::readClientAttributeFrom(attributeId, buf);
	}
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

		DS_LOG_INFO_M("GstVideo::doLoadVideo() movieOpen", GSTREAMER_LOG);
		mPimpl->getMovieRef().open(filename, generateVideoBuffer, false, mIsTransparent, videoWidth, videoHeight);

		setMovieLooping();
		//mMovie.play();
		setMovieVolume();
		if (type == VideoMetaCache::AUDIO_TYPE) {
			// Otherwise I am permanently muted
			mInternalMuted = false;
		} else {
			mInternalMuted = true;
		}
		mPimpl->getMovieRef().setVideoCompleteCallback([this](GStreamerWrapper* video){ mPimpl->handleVideoComplete(video); });
		setStatus(Status::STATUS_PLAYING);

	} catch (std::exception const& ex) {
		DS_DBG_CODE(std::cout << "ERROR GstVideo::doLoadVideo() ex=" << ex.what() << std::endl);
		return;
	}

	if (mPimpl->getMovieRef().getWidth() < 1.0f || mPimpl->getMovieRef().getHeight() < 1.0f) {
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
	markAsDirty(FILENAME_DIRTY);
}

void GstVideo::setStatus(const int code) {
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusDirty = true;
}

void GstVideo::setMovieVolume() {
	if (mMuted || mInternalMuted) {
		mPimpl->getMovieRef().setVolume(0.0f);
	} else {
		mPimpl->getMovieRef().setVolume(mVolume);
	}
}

void GstVideo::setMovieLooping() {
	if (mLooping) {
		mPimpl->getMovieRef().setLoopMode(LOOP);
	} else {
		mPimpl->getMovieRef().setLoopMode(NO_LOOP);
	}
}

void GstVideo::unloadVideo(const bool clearFrame) {
	mPimpl->getMovieRef().stop();
	mPimpl->getMovieRef().close();
	mFilename.clear();
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

void GstVideo::triggerVideoCompleteCallback()
{
	if (mVideoCompleteCallback) {
		mVideoCompleteCallback(this);
	}
}

void GstVideo::setAutoStart( const bool doAutoStart ) {
	// do not check for mAutoStart == doAutoStart. There is no
	// correlation between them. mAutoStart is a cache value. Might be out of sync.
	mAutoStart = doAutoStart;
	mPimpl->getMovieRef().setStartPlaying(mAutoStart);
}

bool GstVideo::getAutoStart() const
{
	return mAutoStart;
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
	mPimpl->getMovieRef().stopOnLoopComplete();
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
