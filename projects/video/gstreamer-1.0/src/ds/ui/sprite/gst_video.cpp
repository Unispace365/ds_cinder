#include "gst_video.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/data/resource_list.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "gstreamer/GStreamerWrapper.h"
#include "gstreamer/gstreamer_env_check.h"
#include "gstreamer/video_meta_cache.h"

#include <cinder/gl/Fbo.h>

#include <mutex>

using namespace ci;
using namespace gstwrapper;

namespace {
	static ds::gstreamer::EnvCheck  ENV_CHECK;
	ds::ui::VideoMetaCache          CACHE("gstreamer");
	const ds::BitMask               GSTREAMER_LOG = ds::Logger::newModule("gstreamer");
	template<typename T> void       noop(T) { /* no op */ };
	void                            noop()  { /* no op */ };
}
namespace ds {
namespace ui {

GstVideo& GstVideo::makeVideo(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::GstVideo>([&e]()->ds::ui::GstVideo*{ return new ds::ui::GstVideo(e); }, parent);
}

/**
 * \class ds::ui::sprite::Video static
 */
GstVideo::GstVideo(SpriteEngine& engine)
	: Sprite(engine)
	, mGstreamerWrapper(new gstwrapper::GStreamerWrapper())
	, mLooping(false)
	, mMuted(false)
	, mOutOfBoundsMuted(true)
	, mVolume(1.0f)
	, mStatusChanged(true)
	, mStatusFn(noop<const Status&>)
	, mVideoCompleteFn(noop)
	, mShouldPlay(false)
	, mAutoStart(false)
	, mShouldSync(false)
	, mFilename("")
	, mStatus(Status::STATUS_STOPPED)
	, mPlaySingleFrame(false)
	, mPlaySingleFrameFunction(nullptr)
	, mDrawable(false)
	, mVideoSize(0, 0)
	, mNetHandler(*this)
	, mAutoExtendIdle(false)
	, mGenerateAudioBuffer(false)
{
	mBlobType = GstVideoNet::mBlobType;

	setUseShaderTextuer(true);
	setTransparent(false);
}

GstVideo::~GstVideo() {
	if(mGstreamerWrapper){
		delete mGstreamerWrapper;
		mGstreamerWrapper = nullptr;
	}
}

void GstVideo::updateServer(const UpdateParams &up){
	Sprite::updateServer(up);

	mGstreamerWrapper->update();

	checkStatus();
	checkOutOfBounds();

	if(mAutoExtendIdle && mStatus == Status::STATUS_PLAYING){
		mEngine.resetIdleTimeOut();
	}
}

void GstVideo::generateAudioBuffer(bool enableAudioBuffer)
{
	 mGenerateAudioBuffer = enableAudioBuffer; 
}

void GstVideo::updateClient(const UpdateParams& up){
	Sprite::updateClient(up);

	mGstreamerWrapper->update();

	checkStatus();
}

void GstVideo::drawLocalClient(){
	if(!mGstreamerWrapper) return;

	if(mGstreamerWrapper->hasVideo() && mGstreamerWrapper->isNewVideoFrame()){

		if(mGstreamerWrapper->getWidth() != mVideoSize.x){
			DS_LOG_WARNING_M("Different sizes detected for video and texture. Do not change the size of a video sprite, use setScale to enlarge. Widths: " << getWidth() << " " << mGstreamerWrapper->getWidth(), GSTREAMER_LOG);
			unloadVideo();
		} else {
			ci::Surface video_surface(
				mGstreamerWrapper->getVideo(),
				mVideoSize.x,
				mVideoSize.y,
				mVideoSize.x * 4, // BGRA: therefore there is 4x8 bits per pixel, therefore 4 bytes per pixel.
				ci::SurfaceChannelOrder::BGRA);

			if(video_surface.getData()){
				mFrameTexture.update(video_surface);
				mDrawable = true;
			}

			if(mPlaySingleFrame){
				stop();
				mPlaySingleFrame = false;
				if(mPlaySingleFrameFunction) mPlaySingleFrameFunction();
				mPlaySingleFrameFunction = nullptr;
			}
		}
	}

	if(mFrameTexture && mDrawable){
		if(getPerspective()){
			mFrameTexture.setFlipped(true);
		} 
			
		ci::gl::draw(mFrameTexture);
	}
}

void GstVideo::setSize( float width, float height ){
	setScale( width / getWidth(), height / getHeight() );
}

GstVideo& GstVideo::loadVideo(const std::string& filename){
	if (mFilename == filename){
		return *this;
	}

	const std::string _filename(ds::Environment::expand(filename));

	if (_filename.empty()){
		DS_LOG_WARNING_M("GstVideo::loadVideo recieved a blank filename. Cancelling load.", GSTREAMER_LOG);
		return *this;
	}

	doLoadVideo(filename);
	markAsDirty(mNetHandler.mPathDirty);
	return *this;
}

GstVideo &GstVideo::setResourceId(const ds::Resource::Id &resourceId){
	try	{
		ds::Resource res;
		if (mEngine.getResources().get(resourceId, res)){
			setResource(res);
		}
	} catch (const std::exception& ex)	{
		DS_LOG_WARNING_M("GstVideo::loadVideo() ex=" << ex.what(), GSTREAMER_LOG);
	}

	return *this;
}

GstVideo& GstVideo::setResource(const ds::Resource& resource){
	Sprite::setSizeAll(resource.getWidth(), resource.getHeight(), mDepth);
	loadVideo(resource.getAbsoluteFilePath());
	return *this;
}

void GstVideo::setLooping(const bool on){
	mLooping = on;
	applyMovieLooping();
	markAsDirty(mNetHandler.mLoopingDirty);
}

bool GstVideo::getIsLooping() const {
	return mLooping;
}
		
void GstVideo::setMute( const bool doMute ){
	mMuted = doMute;
	applyMovieVolume();
	markAsDirty(mNetHandler.mMuteDirty);
}

bool GstVideo::getIsMuted() const {
	return mMuted;
}

void GstVideo::setVolume(const float volume) {
	if (mVolume == volume) return;

	mVolume = volume;
	applyMovieVolume();

	markAsDirty(mNetHandler.mVolumeDirty);
}

float GstVideo::getVolume() const {
	return mVolume;
}

void GstVideo::play(){
	if (mPlaySingleFrame)
		mPlaySingleFrame = false;

	mGstreamerWrapper->play();

	//If movie not yet loaded, remember to play it later once it has
	if (!mGstreamerWrapper->hasVideo()){
		mShouldPlay = true;
	}

	markAsDirty(mNetHandler.mStatusDirty);
}

void GstVideo::stop(){
	mShouldPlay = false;
	mGstreamerWrapper->stop();
	markAsDirty(mNetHandler.mStatusDirty);
}

void GstVideo::pause(){
	mShouldPlay = false;
	mGstreamerWrapper->pause();
	markAsDirty(mNetHandler.mStatusDirty);
}

bool GstVideo::getIsPlaying() const {
	if (mGstreamerWrapper->getState() == PLAYING)	{
		return true;
	} else {
		return false;
	}
}

double GstVideo::getDuration() const {
	return mGstreamerWrapper->getDurationInMs() / 1000.0;
}

double GstVideo::getCurrentTime() const {
	return mGstreamerWrapper->getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t){
	mGstreamerWrapper->setTimePositionInMs(t * 1000.0);
	markAsDirty(mNetHandler.mPosDirty);
}

double GstVideo::getCurrentPosition() const {
	return mGstreamerWrapper->getPosition();
}

void GstVideo::seekPosition(const double t){
	mGstreamerWrapper->setPosition(t);
	markAsDirty(mNetHandler.mPosDirty);
}

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn){
	mStatusFn = fn;
}

void GstVideo::doLoadVideo(const std::string &filename){
	if (filename.empty()) return;
	
	VideoMetaCache::Type		type(VideoMetaCache::ERROR_TYPE);

	try	{
		int						videoWidth = static_cast<int>(getWidth());
		int						videoHeight = static_cast<int>(getHeight());
		double					videoDuration(0.0f);
		bool					generateVideoBuffer = true;

		CACHE.getValues(filename, type, videoWidth, videoHeight, videoDuration);
		
		if (type == VideoMetaCache::AUDIO_TYPE)
		{
			generateVideoBuffer = false;
			mOutOfBoundsMuted = false;
		}

		DS_LOG_INFO_M("GstVideo::doLoadVideo() movieOpen", GSTREAMER_LOG);
		mGstreamerWrapper->open(filename, generateVideoBuffer, mGenerateAudioBuffer, true, videoWidth, videoHeight);

		mVideoSize.x = mGstreamerWrapper->getWidth();
		mVideoSize.y = mGstreamerWrapper->getHeight();
		Sprite::setSizeAll(static_cast<float>(mVideoSize.x), static_cast<float>(mVideoSize.y), mDepth);

		applyMovieLooping();
		applyMovieVolume();

		mGstreamerWrapper->setVideoCompleteCallback([this](GStreamerWrapper*){ 
			if(mVideoCompleteFn) mVideoCompleteFn();
		});

		setStatus(Status::STATUS_PLAYING);

	} catch (std::exception const& ex)	{
		DS_LOG_ERROR_M("GstVideo::doLoadVideo() ex=" << ex.what(), GSTREAMER_LOG);
		return;
	}

	if (mGstreamerWrapper->getWidth() < 1.0f || mGstreamerWrapper->getHeight() < 1.0f){
		if (type != VideoMetaCache::AUDIO_TYPE)	{
			DS_LOG_WARNING_M("GstVideo::doLoadVideo() Video is too small to be used or didn't load correctly! "
				<< filename	<< " " << getWidth() << " "	<< getHeight(), GSTREAMER_LOG);
		}
		return;
	} else {
		ci::gl::Texture::Format fmt;
		mFrameTexture = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);

		mFilename = filename;
	}
}

void GstVideo::setStatus(const int code){
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusChanged = true;

	markAsDirty(mNetHandler.mStatusDirty);
}

void GstVideo::applyMovieVolume(){
	if (mMuted || mOutOfBoundsMuted)	{
		mGstreamerWrapper->setVolume(0.0f);
	} else {
		mGstreamerWrapper->setVolume(mVolume);
	}
}

void GstVideo::applyMovieLooping(){
	if (mLooping)
	{
		mGstreamerWrapper->setLoopMode(LOOP);
	}
	else
	{
		mGstreamerWrapper->setLoopMode(NO_LOOP);
	}
}

void GstVideo::unloadVideo(const bool clearFrame){
	mGstreamerWrapper->stop();
	mGstreamerWrapper->close();
	mFilename.clear();

	if (clearFrame)	{
		mFrameTexture.reset();
	}
}

void GstVideo::setVideoCompleteCallback( const std::function<void()> &func ){
	mVideoCompleteFn = func;
}

void GstVideo::setAutoStart( const bool doAutoStart ) {
	// do not check for mAutoStart == doAutoStart. There is no
	// correlation between them. mAutoStart is a cache value. Might be out of sync.
	mAutoStart = doAutoStart;
	mGstreamerWrapper->setStartPlaying(mAutoStart);
}

bool GstVideo::getAutoStart() const {
	return mAutoStart;
}

void GstVideo::stopAfterNextLoop(){
	mGstreamerWrapper->stopOnLoopComplete();
}

void GstVideo::checkStatus(){
	if (mStatusChanged){
		mStatusFn(mStatus);
		mStatusChanged = false;
	}

	if (mGstreamerWrapper->getState() == STOPPED && mStatus != Status::STATUS_STOPPED){
		setStatus(Status::STATUS_STOPPED);
	} else if (mGstreamerWrapper->getState() == PLAYING	&& mStatus != Status::STATUS_PLAYING){
		setStatus(Status::STATUS_PLAYING);
	} else if (mGstreamerWrapper->getState() == PAUSED && mStatus != Status::STATUS_PAUSED){
		setStatus(Status::STATUS_PAUSED);
	}
	
	if (mStatus != Status::STATUS_PLAYING && mGstreamerWrapper->hasVideo() && mShouldPlay){
		play();
		mShouldPlay = false;
	}
}

void GstVideo::checkOutOfBounds() {
	if (!inBounds()){
		if (!mOutOfBoundsMuted)	{
			mGstreamerWrapper->setVolume(0.0f);
			mOutOfBoundsMuted = true;
		}
		return;
	} else if (mOutOfBoundsMuted) {
		mOutOfBoundsMuted = false;
		applyMovieVolume();
	}
}

const GstVideo::Status& GstVideo::getCurrentStatus() const {
	return mStatus;
}

const std::string& GstVideo::getLoadedFilename() const {
	return mFilename;
}

void GstVideo::writeAttributesTo(DataBuffer& buf){
	Sprite::writeAttributesTo(buf);
	mNetHandler.writeAttributesTo(mDirty, buf);
}

void GstVideo::readAttributeFrom(const char id, DataBuffer& buf){
	if (!mNetHandler.readAttributeFrom(id, buf))
		Sprite::readAttributeFrom(id, buf);
}

double GstVideo::getCurrentTimeMs() const {
	return mGstreamerWrapper->getCurrentTimeInMs();
}

void GstVideo::playAFrame(double time_ms, const std::function<void()>& fn){
	mPlaySingleFrame = true;
	mPlaySingleFrameFunction = fn;
	if(!getIsPlaying()) {
		play();
	}

	if(time_ms >= 0.0){
		seekTime(time_ms);
	}
}

bool GstVideo::isPlayingAFrame()const {
	return mPlaySingleFrame;
}

bool GstVideo::getAutoExtendIdle()const {
	return mAutoExtendIdle;
}

void GstVideo::setAutoExtendIdle(const bool doAutoextend){
	mAutoExtendIdle = doAutoextend;
}

GstVideo::Status::Status(int code)
{
	mCode = code;
}

bool GstVideo::Status::operator==(int status) const {
	return mCode == status;
}

bool GstVideo::Status::operator!=(int status) const {
	return mCode != status;
}

} //!namespace ui
} //!namespace ds