#include "gst_video.h"

#include <ds/app/app.h>
#include <ds/app/blob_registry.h>
#include <ds/app/blob_reader.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/data/data_buffer.h>
#include <ds/data/resource_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/computer_info.h>
#include <ds/app/engine/engine_io_defs.h>

#include "gstreamer/gstreamer_wrapper.h"
#include "gstreamer/gstreamer_env_check.h"
#include "gstreamer/video_meta_cache.h"

#include <cinder/gl/Fbo.h>

#include <gst/net/gstnettimeprovider.h>

#include <mutex>

#include  <ds/network/network_info.h>

//#include "network_info.h"

using namespace ci;
using namespace gstwrapper;

namespace ds {
namespace ui {

namespace {
static ds::gstreamer::EnvCheck  ENV_CHECK;
ds::ui::VideoMetaCache          CACHE("gstreamer-2");
const ds::BitMask               GSTREAMER_LOG = ds::Logger::newModule("gstreamer");
template<typename T> void       noop(T) { /* no op */ };
void                            noop()  { /* no op */ };

static int drawcount = 0;

static std::string yuv_vert =
"varying   vec2 gsvTexCoord;"
"void main(){ "
"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;"
"gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;"
"gl_FrontColor = gl_Color;"
"gsvTexCoord = gl_TexCoord[0].xy;"
"}";

	const static std::string yuv_frag =
		"precision mediump float;"
		"uniform sampler2D gsuTexture0;"
		"uniform sampler2D gsuTexture1;"
		"uniform sampler2D gsuTexture2;"
		"varying vec2 gsvTexCoord;"
		"void main(){"
		"float y = texture2D(gsuTexture0, gsvTexCoord).r;"
		"float u = texture2D(gsuTexture1, gsvTexCoord).r;"
		"float v = texture2D(gsuTexture2, gsvTexCoord).r;"
		"u = u - 0.5;"
		"v = v - 0.5;"
		"gl_FragData[0] = vec4( (y + (1.403 * v)) * 1.1643835 - 0.062745, (y - (0.344 * u) - (0.714 * v)) * 1.1643835 - 0.062745, (y + (1.770 * u)) * 1.1643835 - 0.062745, gl_Color.a);" //  ITU.BT-601 Y’CbCr color space transposed to super space for better contrast

		// Below are other test color spaces, some ok, some poopy
		//"gl_FragData[0] = vec4( y + (1.403 * v), y - (0.344 * u) - (0.714 * v), y + (1.770 * u), gl_Color.a);" //  ITU.BT-601 Y’CbCr color space
		//"gl_FragData[0] = vec4( y + (1.13983 * v), y - (0.39465 * u) - (0.58060 * v), y + (2.03211 * u), gl_Color.a);" // 601 color space
		//"gl_FragData[0] = vec4( y + (1.28033 * v), y - (0.21482 * u) - (0.38059 * v), y + (2.12798 * u), gl_Color.a);" // 709 (wikipedia) color space
		//"gl_FragData[0] = vec4( (y + (1.28033 * v)) * 1.1643835 - 0.062745, (y - (0.21482 * u) - (0.38059 * v)) * 1.1643835 - 0.062745, (y + (2.12798 * u)) * 1.1643835 - 0.062745, gl_Color.a);" // 709 (wikipedia) color space transposed to Super space (0-255, from YUV's 16-235)
		//"gl_FragData[0] = vec4( y + (1.5701 * v), y - (0.1870 * u) - (0.4664 * v), y + (1.8556 * u), gl_Color.a);" //  ITU.BT-709 HDTV studio production in Y’CbCr  color space http://www.poynton.com/PDFs/coloureq.pdf
		//"gl_FragData[0] = vec4( y + (1.5756 * v), y - (0.2253 * u) + (0.50 * v), y + (1.8270 * u), gl_Color.a);" //  SMPTE-240M Y’PbPr color space (totally not right)
		//"gl_FragData[0] = vec4( y + (1.370705 * v), y - (0.698001 * u) - (0.337633 * v), y + (1.732446 * u), gl_Color.a);" // android color space
		//"gl_FragData[0] = vec4( clamp(y + (1.28033 * v), 0, 1), clamp(y - (0.21482 * u) - (0.38059 * v), 0, 1), clamp(y + (2.12798 * u), 0, 1), gl_Color.a);" // 709 clamped color space
		"}";
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {

			e.installSprite([](ds::BlobRegistry& r){ds::ui::GstVideo::installAsServer(r); },
							[](ds::BlobRegistry& r){ds::ui::GstVideo::installAsClient(r); });
		});

	}
	void			doNothing() { }
};

Init		INIT;

char		BLOB_TYPE = 0;

const char mMuteAtt = 81;
const char mStatusAtt = 82;
const char mVolumeAtt = 83;
const char mLoopingAtt = 84;
const char mAutoStartAtt = 85;
const char mPathAtt = 86;
const char mPosAtt = 87;
const char mSyncAtt = 88;
const char mPanAtt = 89;

// Fuck you, 90

const char mFastPosAtt = 91;
const char mUpdateAtt = 92;
const char mUpdateBaseTimeAtt = 93;
const char mUpdateSeekTimeAtt = 94;
const char mSeekAtt = 95;
const char mInstancesAtt = 96;
const char mDoSyncAtt = 97;

const DirtyState& mPosDirty = newUniqueDirtyState();

const DirtyState& mPathDirty = newUniqueDirtyState();

const DirtyState& mMuteDirty = newUniqueDirtyState();
const DirtyState& mStatusDirty = newUniqueDirtyState();
const DirtyState& mUpdateDirty = newUniqueDirtyState();

const DirtyState& mVolumeDirty = newUniqueDirtyState();
const DirtyState& mPanDirty = newUniqueDirtyState();
const DirtyState& mLoopingDirty = newUniqueDirtyState();
const DirtyState& mAutoStartDirty = newUniqueDirtyState();
const DirtyState& mSyncDirty = newUniqueDirtyState();

const DirtyState& mFastPosDirty = newUniqueDirtyState();
const DirtyState& mBaseTimeDirty = newUniqueDirtyState();
const DirtyState& mSeekTimeDirty = newUniqueDirtyState();
const DirtyState& mSeekDirty = newUniqueDirtyState();

const DirtyState& mInstancesDirty = newUniqueDirtyState();
const DirtyState& mDoSyncDirty = newUniqueDirtyState();

}

GstVideo& GstVideo::makeVideo(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::GstVideo>([&e]()->ds::ui::GstVideo*{ return new ds::ui::GstVideo(e); }, parent);
}

void GstVideo::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r); });
}

void GstVideo::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<GstVideo>(r); });
}

/**
 * \class ds::ui::sprite::Video static
 */
GstVideo::GstVideo(SpriteEngine& engine)
	: Sprite(engine)
	, mGstreamerWrapper(new gstwrapper::GStreamerWrapper())
	, mLooping(false)
	, mMuted(false)
	, mEngineMuted(false)
	, mOutOfBoundsMuted(true)
	, mAllowOutOfBoundsMuted(true)
	, mVolume(1.0f)
	, mStatusChanged(true)
	, mStatusFn(noop<const Status&>)
	, mVideoCompleteFn(noop)
	, mErrorFn(nullptr)
	, mShouldPlay(false)
	, mAutoStart(false)
	, mShouldSync(false)
	, mFilename("")
	, mStatus(Status::STATUS_STOPPED)
	, mPlaySingleFrame(false)
	, mPlaySingleFrameFunction(nullptr)
	, mDrawable(false)
	, mVideoSize(0, 0)
	, mAutoExtendIdle(false)
	, mGenerateAudioBuffer(false)
	, mColorType(kColorTypeTransparent)
	, mNetPort(-1)
	, mBaseTime(0)
	, mSeekTime(0)
	, mCachedDuration(0)
	, mDoSyncronization(true)
	, mServerOnlyMode(false)
	, mServerPosition(0)
	, mServerDuration(0)
	, mServerPlayStatus(Status::STATUS_STOPPED)
	, mPan(0.0f)
	, mStreaming(false)
{
	mBlobType = BLOB_TYPE;

	try {
	} catch(const std::exception &e) {
		DS_LOG_WARNING("Could not load & compile shader for the video:" << e.what());
	}
	
	mEngineMuted = mEngine.getMute();

	setTransparent(false);
	setUseShaderTextuer(true);
}

GstVideo::~GstVideo(){
	if(mGstreamerWrapper){
		delete mGstreamerWrapper;
		mGstreamerWrapper = nullptr;
	}
}

void GstVideo::updateServer(const UpdateParams &up){
	Sprite::updateServer(up);

	mGstreamerWrapper->update();

	if (mGstreamerWrapper->isNewLoop()){
		mGstreamerWrapper->clearNewLoop();
		markAsDirty(mBaseTimeDirty);
		markAsDirty(mSeekDirty);
	} else
	//m_StartTime is used for pause/resume operations
	if (mGstreamerWrapper->isPlayFromPause() )	{
		mGstreamerWrapper->clearPlayFromPause();
		mBaseTime = mGstreamerWrapper->getBaseTime();
		mSeekTime = mGstreamerWrapper->getSeekTime();

		markAsDirty(mBaseTimeDirty);
		markAsDirty(mSeekTimeDirty);
	}
	
	//check wrapper for new time sync.  If new, mark as dirty
	//Don't try to sync base-time while fast seeking
	else if (mGstreamerWrapper->getBaseTime() != mBaseTime ) {
		mBaseTime = mGstreamerWrapper->getBaseTime();
		markAsDirty(mBaseTimeDirty);
	}	
	
	checkStatus();
	checkOutOfBounds();

	if(mAutoExtendIdle && mStatus == Status::STATUS_PLAYING){
		mEngine.resetIdleTimeOut();
	}
}

void GstVideo::generateAudioBuffer(bool enableAudioBuffer ){
	 mGenerateAudioBuffer = enableAudioBuffer; 
}

void GstVideo::setVerboseLogging(const bool doVerbose){
	mGstreamerWrapper->setVerboseLogging(doVerbose);
}

float GstVideo::getVideoPlayingFramerate(){
	if(mBufferUpdateTimes.size() < 2) return 0.0f;
	float deltaTime = (float)(mBufferUpdateTimes.back() - mBufferUpdateTimes.front()) / 1000000.0f;
	return (float)(mBufferUpdateTimes.size() - 1) / deltaTime;
}

void GstVideo::setAutoSynchronize(const bool doSync){
	mDoSyncronization = doSync;
	markAsDirty(mDoSyncDirty);
}

void GstVideo::setPlayableInstances(const std::vector<std::string>& instanceNames){
	mPlayableInstances = instanceNames;
	markAsDirty(mInstancesDirty);
}

void GstVideo::updateClient(const UpdateParams& up){
	Sprite::updateClient(up);

	mGstreamerWrapper->update();

	checkStatus();
	checkOutOfBounds();

	// The code below this is only for synchronization
	if(!mDoSyncronization) return;

	//If client tried to sync with server clock before it was ready, it can miss its opportunity
	//and stay un-synced until the next 'event' (scrub/pause, loop).  
	//This code detects that condition, and re-initiates contact.
	if (mGstreamerWrapper->getBaseTime() > mGstreamerWrapper->getNetClockTime() &&
		mGstreamerWrapper->getState() == PLAYING){
		play();
	}
}

void GstVideo::drawLocalClient(){
	if (!mGstreamerWrapper){
		DS_LOG_WARNING("Gstreamer wrapper not available");
		return;
	}
	if(mGstreamerWrapper->hasVideo() && mGstreamerWrapper->isNewVideoFrame()){

		if(mGstreamerWrapper->getWidth() != mVideoSize.x){
			DS_LOG_WARNING_M("Different sizes detected for video and texture. Do not change the size of a video sprite, use setScale to enlarge. Widths: " << getWidth() << " " << mGstreamerWrapper->getWidth(), GSTREAMER_LOG);
			unloadVideo();
		} else {
			int videoDepth = mVideoSize.x * 4; // BGRA: therefore there is 4x8 bits per pixel, therefore 4 bytes per pixel.
			ci::SurfaceChannelOrder co = ci::SurfaceChannelOrder::BGRA;
			if(mColorType == kColorTypeSolid){
				videoDepth = mVideoSize.x * 3;
				co = ci::SurfaceChannelOrder::BGR;
			} else if(mColorType == kColorTypeShaderTransform){
				videoDepth = mVideoSize.x;
				co = ci::SurfaceChannelOrder::CHAN_RED;
			}

			unsigned char * dat = nullptr;
			Surface8u* video_surface = nullptr;

			dat = mGstreamerWrapper->getVideo();

			if (dat){
				if (mColorType == kColorTypeShaderTransform ){

					ci::Channel8u yChannel(mVideoSize.x, mVideoSize.y, mVideoSize.x, 1, dat);
					ci::Channel8u uChannel(mVideoSize.x / 2, mVideoSize.y / 2, mVideoSize.x / 2, 1, dat + mVideoSize.x * mVideoSize.y);
					ci::Channel8u vChannel(mVideoSize.x / 2, mVideoSize.y / 2, mVideoSize.x / 2, 1, dat + mVideoSize.x * mVideoSize.y + mVideoSize.x * (mVideoSize.y / 4));

					mFrameTexture.update(yChannel, ci::Area(0, 0, mVideoSize.x, mVideoSize.y));
					mUFrameTexture.update(uChannel, ci::Area(0, 0, mVideoSize.x / 2, mVideoSize.y / 2));
					mVFrameTexture.update(vChannel, ci::Area(0, 0, mVideoSize.x / 2, mVideoSize.y / 2));
				} else {
					ci::Surface video_surface(dat, mVideoSize.x, mVideoSize.y, videoDepth, co);
					mFrameTexture.update(video_surface);
				}

				mDrawable = true;
			}

			if (mPlaySingleFrame){
				stop();
				mPlaySingleFrame = false;
				if (mPlaySingleFrameFunction) mPlaySingleFrameFunction();
				mPlaySingleFrameFunction = nullptr;
			}

			mBufferUpdateTimes.push_back(Poco::Timestamp().epochMicroseconds());
			if (mBufferUpdateTimes.size() > 10){
				mBufferUpdateTimes.erase(mBufferUpdateTimes.begin());
			}
		}
	}

	if (mFrameTexture && mDrawable){
		if (mColorType == kColorTypeShaderTransform){
			ci::gl::disableDepthRead();
			ci::gl::disableDepthWrite();
			if (mSpriteShader.getName().compare("yuv_colorspace_conversion") == 0){

				if (mFrameTexture) mFrameTexture.bind(2);
				if (mUFrameTexture) mUFrameTexture.bind(3);
				if (mVFrameTexture) mVFrameTexture.bind(4);

			}
			if(getPerspective()){
				ci::gl::drawSolidRect(ci::Rectf(0.0f, mHeight, mWidth, 0.0f));
			} else {
				ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight));
			}

		} else {
			if (getPerspective()){
			 	mFrameTexture.setFlipped(true);
			}
			if (mFrameTexture) mFrameTexture.bind(0);
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, mWidth, mHeight));
		}


		if (mColorType == kColorTypeShaderTransform){
			if (mSpriteShader.getName().compare("yuv_colorspace_conversion") == 0){
				if (mFrameTexture) mFrameTexture.unbind(2);
				if (mUFrameTexture) mUFrameTexture.unbind(3);
				if (mVFrameTexture) mVFrameTexture.unbind(4);
			}
		} else {
			if (mFrameTexture) mFrameTexture.unbind();
		}
	} 
}

void GstVideo::setSize( float width, float height ){
	setScale( width / getWidth(), height / getHeight() );
}

GstVideo& GstVideo::loadVideo(const std::string& filename){
	const std::string _filename(ds::Environment::expand(filename));

	if (_filename.empty()){
		DS_LOG_WARNING_M("GstVideo::loadVideo received a blank filename. Cancelling load.", GSTREAMER_LOG);
		return *this;
	}

	doLoadVideo(_filename, filename);
	markAsDirty(mPathDirty);
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
	if(resource.getType() == ds::Resource::VIDEO_TYPE){
		Sprite::setSizeAll(resource.getWidth(), resource.getHeight(), mDepth);
		loadVideo(resource.getAbsoluteFilePath());
	} else if(resource.getType() == ds::Resource::VIDEO_STREAM_TYPE) {
		std::string path = resource.getAbsoluteFilePath();
		startStream(path, resource.getWidth(), resource.getHeight());
	}
	return *this;
}

void GstVideo::setAutoRestartStream(bool autoRestart){
	if(mGstreamerWrapper){
		mGstreamerWrapper->setAutoRestartStream(autoRestart);
	}
}

bool GstVideo::thisInstancePlayable(){
	bool doLoad = true;
	if(!mPlayableInstances.empty()){
		std::string thisInstance = mEngine.getAppInstanceName();
		doLoad = false;
		for(auto it = mPlayableInstances.begin(); it < mPlayableInstances.end(); ++it){
			if((*it) == thisInstance){
				doLoad = true;
				break;
			}
		}
	}
	return doLoad;
}

void GstVideo::doLoadVideo(const std::string &filename, const std::string &portable_filename){
	if(filename.empty()){
		DS_LOG_WARNING_M("doLoadVideo aborting loading a video because of a blank filename.", GSTREAMER_LOG);
		if(mErrorFn) mErrorFn("Did not load a video because there was no filename.");
		return;
	}

	mStreaming = false;

	// This allows apps to only load videos on certain client instances
	// The default is to load everything everywhere
	// If nothing has been specified, then load everywhere
	bool doLoad = thisInstancePlayable();

	VideoMetaCache::Type		type(VideoMetaCache::ERROR_TYPE);

	try	{
		int						videoWidth = static_cast<int>(getWidth());
		int						videoHeight = static_cast<int>(getHeight());
		bool					generateVideoBuffer = true;
		std::string				colorSpace = "";

		CACHE.getValues(filename, type, videoWidth, videoHeight, mCachedDuration, colorSpace);

		mFilename = filename;
		mPortableFilename = portable_filename;

		if(!doLoad){
			mVideoSize.x = videoWidth;
			mVideoSize.y = videoHeight;
			setSizeAll(static_cast<float>(videoWidth), static_cast<float>(videoHeight), mDepth);
			setStatus(Status::STATUS_PLAYING);
			mServerOnlyMode = true;
			return;
		}

		mServerOnlyMode = false;

		if(mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess() > 800.0f){
			setSizeAll(static_cast<float>(videoWidth), static_cast<float>(videoHeight), mDepth);
			DS_LOG_WARNING_M("doLoadVideo aborting loading a video because we're almost out of memory", GSTREAMER_LOG);
			if(mErrorFn) mErrorFn("Did not load a video because the system ran out of memory.");
			return;
		}

		if(type == VideoMetaCache::AUDIO_TYPE){
			generateVideoBuffer = false;
			mOutOfBoundsMuted = false;
		}
		// if the video was set previously, clear out the shader so we don't multiple-set the shader
		removeShaders();
		setBaseShader(Environment::getAppFolder("data/shaders"), "base");

		mDrawable = false;

		ColorType theColor = ColorType::kColorTypeTransparent;
		if(colorSpace == "4:2:0"){
			theColor = ColorType::kColorTypeShaderTransform;
			std::string shaderName("yuv_colorspace_conversion");
			addNewMemoryShader(yuv_vert, yuv_frag, shaderName, true);
			ds::gl::Uniform uniform;

			uniform.setInt("gsuTexture0", 2);
			uniform.setInt("gsuTexture1", 3);
			uniform.setInt("gsuTexture2", 4);
			setShadersUniforms("yuv_colorspace_conversion", uniform);
		}

		mColorType = theColor;
		DS_LOG_INFO_M("GstVideo::doLoadVideo() movieOpen: " << filename, GSTREAMER_LOG);
		mGstreamerWrapper->open(filename, generateVideoBuffer, mGenerateAudioBuffer, theColor, videoWidth, videoHeight);

		mVideoSize.x = mGstreamerWrapper->getWidth();
		mVideoSize.y = mGstreamerWrapper->getHeight();
		Sprite::setSizeAll(static_cast<float>(mVideoSize.x), static_cast<float>(mVideoSize.y), mDepth);

		applyMovieLooping();
		applyMovieVolume();
		applyMoviePan(mPan);

		mGstreamerWrapper->setVideoCompleteCallback([this](GStreamerWrapper*){
			if(mVideoCompleteFn) mVideoCompleteFn();
		});

		// TODO: add error callbacks to the server?
		mGstreamerWrapper->setErrorMessageCallback([this](const std::string& msg){
			if(mErrorFn) mErrorFn(msg);
		});

		if(mDoSyncronization && mEngine.getMode() == ds::ui::SpriteEngine::CLIENTSERVER_MODE){
			setNetClock();
		}

		setStatus(Status::STATUS_PLAYING);

	} catch(std::exception const& ex)	{
		DS_LOG_ERROR_M("GstVideo::doLoadVideo() ex=" << ex.what(), GSTREAMER_LOG);
		return;
	}

	if(mGstreamerWrapper->getWidth() < 1.0f || mGstreamerWrapper->getHeight() < 1.0f){
		if(type != VideoMetaCache::AUDIO_TYPE)	{
			DS_LOG_WARNING_M("GstVideo::doLoadVideo() Video is too small to be used or didn't load correctly! "
							 << filename << " " << getWidth() << " " << getHeight(), GSTREAMER_LOG);
		}
		return;
	} else {
		ci::gl::Texture::Format fmt;

		if(mColorType == kColorTypeShaderTransform){
			fmt.setInternalFormat(GL_LUMINANCE);
			mFrameTexture  = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);
			mUFrameTexture = ci::gl::Texture(static_cast<int>(getWidth() / 2.0f), static_cast<int>(getHeight() / 2.0f), fmt);
			mVFrameTexture = ci::gl::Texture(static_cast<int>(getWidth() / 2.0f), static_cast<int>(getHeight() / 2.0f), fmt);
		} else {
			mFrameTexture  = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);
		}

	}
}

void GstVideo::startStream(const std::string& streamingPipeline, const float videoWidth, const float videoHeight){
	if(streamingPipeline.empty()){
		DS_LOG_WARNING_M("GstVideo::startStream aborting starting streaming because of a blank pipeline.", GSTREAMER_LOG);
		return;
	}

	if(videoWidth < 16.0f || videoHeight < 16.0f){
		DS_LOG_WARNING_M("GstVideo::startStream aborting streaming cause of too small a size. Must be > 16 pixels on a side.", GSTREAMER_LOG);
		return;
	}

	removeShaders();
	setBaseShader(Environment::getAppFolder("data/shaders"), "base");

	mDrawable = false;
	mStreaming = true;
	mFilename = streamingPipeline;
	mPortableFilename = streamingPipeline;
	mVideoSize.x = (int)floorf(videoWidth);
	mVideoSize.y = (int)floorf(videoHeight);
	markAsDirty(mPathDirty);

	bool doLoad = thisInstancePlayable();
	if(!doLoad){
		setSizeAll(videoWidth, videoHeight, mDepth);
		setStatus(Status::STATUS_PLAYING);
		mServerOnlyMode = true;
		return;
	}
		
	mOutOfBoundsMuted = true;
	mColorType = ColorType::kColorTypeShaderTransform;
	std::string name("yuv_colorspace_conversion");
	addNewMemoryShader(yuv_vert, yuv_frag, name, true);
	ds::gl::Uniform uniform;

	uniform.setInt("gsuTexture0", 2);
	uniform.setInt("gsuTexture1", 3);
	uniform.setInt("gsuTexture2", 4);
	setShadersUniforms("yuv_colorspace_conversion", uniform);

	DS_LOG_INFO_M("GstVideo::startStream() " << streamingPipeline, GSTREAMER_LOG);
	if(!mGstreamerWrapper->openStream(streamingPipeline, (int)floorf(videoWidth), (int)floorf(videoHeight))){
		DS_LOG_WARNING_M("GstVideo::startStream() aborting cause of a problem.", GSTREAMER_LOG);
		return;
	}

	mVideoSize.x = mGstreamerWrapper->getWidth();
	mVideoSize.y = mGstreamerWrapper->getHeight();

	Sprite::setSizeAll(static_cast<float>(mVideoSize.x), static_cast<float>(mVideoSize.y), mDepth);

	applyMovieVolume();

	setStatus(Status::STATUS_PLAYING);

	mGstreamerWrapper->setErrorMessageCallback([this](const std::string& msg){
		if(mErrorFn) mErrorFn(msg);
	});

	ci::gl::Texture::Format fmt;
	if(mColorType == kColorTypeShaderTransform){
		fmt.setInternalFormat(GL_LUMINANCE);
		mFrameTexture = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);
		mUFrameTexture = ci::gl::Texture(static_cast<int>(getWidth() / 2.0f), static_cast<int>(getHeight() / 2.0f), fmt);
		mVFrameTexture = ci::gl::Texture(static_cast<int>(getWidth() / 2.0f), static_cast<int>(getHeight() / 2.0f), fmt);
	} else {
		mFrameTexture = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);
	}
}

void GstVideo::setLooping(const bool on){
	mLooping = on;
	applyMovieLooping();
	markAsDirty(mLoopingDirty);
}

bool GstVideo::getIsLooping() const {
	return mLooping;
}
		
void GstVideo::setMute( const bool doMute ){
	mMuted = doMute;
	applyMovieVolume();
	markAsDirty(mMuteDirty);
}

bool GstVideo::getIsMuted() const {
	return mMuted;
}

void GstVideo::setVolume(const float volume) {
	if (mVolume == volume) return;

	mVolume = volume;
	applyMovieVolume();

	markAsDirty(mVolumeDirty);
}

float GstVideo::getVolume() const {
	return mVolume;
}

void GstVideo::setPan(const float pan) {
	if (mPan == pan) return;

	mPan = pan;
	applyMoviePan(mPan);
	markAsDirty(mPanDirty);
}

float GstVideo::getPan() const {
	return mPan;
}

void GstVideo::play(){
	mServerPlayStatus = Status::STATUS_PLAYING;
	if(mServerOnlyMode){
		markAsDirty(mStatusDirty);
		return;
	} else {
		mGstreamerWrapper->play();
	}

	//If movie not yet loaded, remember to play it later once it has
	if (!mGstreamerWrapper->hasVideo()){
		mShouldPlay = true;
	}

	markAsDirty(mBaseTimeDirty);
	markAsDirty(mSeekTimeDirty);
}

void GstVideo::stop(){
	mServerPlayStatus = Status::STATUS_STOPPED;
	mShouldPlay = false;
	if(mServerOnlyMode){
		markAsDirty(mStatusDirty);
	} else {
		mGstreamerWrapper->stop();
	}
}

void GstVideo::pause(){
	mServerPlayStatus = Status::STATUS_PAUSED;
	mShouldPlay = false;
	if(mServerOnlyMode){
		markAsDirty(mStatusDirty);
	} else {
		mGstreamerWrapper->pause();
	}
	 
	markAsDirty(mSeekTimeDirty);
}

bool GstVideo::getIsPlaying() const {
	if(mServerOnlyMode){
		return mServerPlayStatus == Status::STATUS_PLAYING;
	}

	if (mGstreamerWrapper->getState() == PLAYING){
		return true;
	} else {
		return false;
	}
}

double GstVideo::getDuration() const {
	if(mServerOnlyMode){
		return mServerDuration;
	}

	double gstDur = mGstreamerWrapper->getDurationInMs();

	// if gstreamer hasn't found the duration or the video hasn't been loaded yet, use the cached value
	if(gstDur < 1){
		gstDur = mCachedDuration;
	} else {
		gstDur /= 1000.0;
	}
	return gstDur;
}

double GstVideo::getCurrentTime() const {
	if(mServerOnlyMode){
		return mServerPosition * mServerDuration;
	}
	return mGstreamerWrapper->getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t){
	if(mServerOnlyMode){
		mServerPosition = t / mServerDuration;
	} else {
		mGstreamerWrapper->setTimePositionInMs(t * 1000.0);
	}
	markAsDirty(mPosDirty);
}

double GstVideo::getCurrentPosition() const {
	if(mServerOnlyMode){
		return mServerPosition;
	}
	return mGstreamerWrapper->getPosition();
}

void GstVideo::seekPosition(const double t){
	if(mServerOnlyMode){
		mServerPosition = t;
	} else {
		mGstreamerWrapper->setPosition(t);
	}
	markAsDirty(mPosDirty);
}

void GstVideo::setStatus(const int code){
	mServerPlayStatus = code;

	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusChanged = true;

	markAsDirty(mStatusDirty);
}

void GstVideo::applyMovieVolume(){
	if (mMuted || (mAllowOutOfBoundsMuted && mOutOfBoundsMuted) || mEngineMuted) {
		mGstreamerWrapper->setVolume(0.0f);
	} else {
		mGstreamerWrapper->setVolume(mVolume);
	}
}

void GstVideo::applyMoviePan(const float pan){
	mGstreamerWrapper->setPan(pan);
}

void GstVideo::applyMovieLooping(){
	if(mLooping){
		mGstreamerWrapper->setLoopMode(LOOP);
	} else {
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

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn){
	mStatusFn = fn;
}

void GstVideo::setErrorCallback(const std::function<void(const std::string& errorMessage)>& func){
	mErrorFn = func;
}

void GstVideo::setVideoCompleteCallback(const std::function<void()> &func){
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
void GstVideo::setNetClock(){
	if(!mDoSyncronization || mStreaming) return;

	if (mIpAddress.empty()) {
		ds::network::networkInfo* Networki;
		Networki = new ds::network::networkInfo();

		mIpAddress = Networki->getAddress();
	}
	if (mEngine.getMode() == ds::ui::SpriteEngine::STANDALONE_MODE){
		// NOTHIN
	} else if(mEngine.getMode() == ds::ui::SpriteEngine::SERVER_MODE){
		mServerOnlyMode = true;
		//DS_LOG_WARNING_M("Gstreamer net sync not implemented in Server only mode. Use ClientServer insteand.", GSTREAMER_LOG);
	} else if(mEngine.getMode() == ds::ui::SpriteEngine::CLIENTSERVER_MODE){
		//Read port from settings file if available.  Otherwise, pick default.
		static int newPort = mEngine.getSettings("layout").getInt("gstVideo:netclock:port", 0, 0);
		if (newPort == 0){
			newPort = DEFAULT_PORT;
		}
		//Assign new port if this is a new instance.  Otherwise, keep the same
		if (mNetPort < 0){
			newPort++;
		}
		mNetPort = newPort-1;

		mGstreamerWrapper->setServerNetClock(true, mIpAddress, mNetPort, mNetClock, mBaseTime);

		markAsDirty(mSyncDirty);
	} else if(mEngine.getMode() == ds::ui::SpriteEngine::CLIENT_MODE){
		//Wait for server to initiate contact
		mGstreamerWrapper->setClientNetClock(false, mIpAddress, mNetPort, mNetClock, mBaseTime);
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
	if(mServerOnlyMode){
		return mServerPlayStatus;
	}
	return mStatus;
}

const std::string& GstVideo::getLoadedFilename() const {
	return mFilename;
}

void GstVideo::writeAttributesTo(DataBuffer& buf){
	Sprite::writeAttributesTo(buf);

	// Add the "do sync" property before setting the path, so the client knows if it needs to sync or not
	if(mDirty.has(mDoSyncDirty)){
		buf.add(mDoSyncAtt);
		buf.add(mDoSyncronization);
	}

	// Add the instances before setting the path in case the client shouldn't play it
	if(mDirty.has(mInstancesDirty)){
		int numOfInstances = (int)mPlayableInstances.size();
		buf.add(mInstancesAtt);
		buf.add(numOfInstances);
		for(auto it = mPlayableInstances.begin(); it < mPlayableInstances.end(); ++it){
			buf.add((*it));
		}
	}

	if (mDirty.has(mPathDirty)){
		buf.add(mPathAtt);
		buf.add(mGenerateAudioBuffer);
		buf.add(mStreaming);
		buf.add(mPortableFilename);
		if(mStreaming){
			buf.add(mVideoSize.x);
			buf.add(mVideoSize.y);
		}
	}

	if (mDirty.has(mAutoStartDirty)){
		buf.add(mAutoStartAtt);
		buf.add(getAutoStart());
	}

	if (mDirty.has(mVolumeDirty)){
		buf.add(mVolumeAtt);
		buf.add(getVolume());
	}

	if (mDirty.has(mLoopingDirty)){
		buf.add(mLoopingAtt);
		buf.add(getIsLooping());
	}

	if (mDirty.has(mVolumeDirty)){
		buf.add(mVolumeAtt);
		buf.add(getVolume());
	}

	if (mDirty.has(mPanDirty)){
		buf.add(mPanAtt);
		buf.add(getPan());  
	}

	if (mDirty.has(mMuteDirty)){
		buf.add(mMuteAtt);
		buf.add(getIsMuted());
	}

	if (mDirty.has(mSyncDirty)){
		buf.add(mSyncAtt);
		buf.add(mNetPort);
		buf.add(mBaseTime);
		buf.add(mNetClock);
		buf.add(mIpAddress);
	}

	if (mDirty.has(mBaseTimeDirty)){
		mBaseTime = mGstreamerWrapper->getBaseTime();
		buf.add(mUpdateBaseTimeAtt);
		buf.add(mBaseTime);
	}

	if (mDirty.has(mSeekTimeDirty)){
		mSeekTime = mGstreamerWrapper->getSeekTime();
		buf.add(mUpdateSeekTimeAtt);
		buf.add(mSeekTime);
	}

	if (mDirty.has(mSeekDirty)){
		mSeekTime = mGstreamerWrapper->getSeekTime();
		buf.add(mSeekAtt);
		buf.add(mSeekTime);
	}

	if(mDirty.has(mStatusDirty)){
		buf.add(mStatusAtt);
		if(mServerOnlyMode){
			buf.add(mServerPlayStatus);
		} else {
			buf.add(getCurrentStatus().mCode);
		}
	}

	//Some things we don't want to  do on initialization.
	auto isReset = mDirty.getMaskValue() & 0x80000000;
	if(!isReset && mDirty.has(mPosDirty)){
		buf.add(mPosAtt);
		buf.add(getCurrentPosition());
	}
}

void GstVideo::readAttributeFrom(const char attrid, DataBuffer& buf){
	if(attrid == mPathAtt) {
		mGenerateAudioBuffer = buf.read<bool>();
		mStreaming = buf.read<bool>();
		auto video_path = buf.read<std::string>();
		if(mStreaming){
			int vidWidth = buf.read<int>();
			int vidHeight = buf.read<int>();
			startStream(video_path, (float)vidWidth, (float)vidHeight);
		} else {
			if(getLoadedFilename() != video_path){
				loadVideo(video_path);
			}
		}
	} else if(attrid == mAutoStartAtt) {
		auto auto_start = buf.read<bool>();

		if(getAutoStart() != auto_start)
			setAutoStart(auto_start);
	} else if(attrid == mLoopingAtt) {
		auto is_looping = buf.read<bool>();
		if(getIsLooping() != is_looping)
			setLooping(is_looping);
	} else if(attrid == mMuteAtt) {
		auto is_muted = buf.read<bool>();
		setMute(is_muted);
	} else if(attrid == mVolumeAtt) {
		auto volume_level = buf.read<float>();
		if(getVolume() != volume_level)
			setVolume(volume_level);
	} else if (attrid == mPanAtt) {
		auto pan = buf.read<float>();
		setPan(pan); 
	} else if (attrid == mPosAtt) {
		auto server_video_pos = buf.read<double>();
		seekPosition(server_video_pos);
	}
	else if (attrid == mStatusAtt) {
		checkStatus();
		auto status_code = buf.read<int>();
		if(getCurrentStatus() != status_code){
			if(status_code == GstVideo::Status::STATUS_PAUSED){
				pause();
			} else if(status_code == GstVideo::Status::STATUS_STOPPED){
				stop();
			} else if(status_code == GstVideo::Status::STATUS_PLAYING){
				play();
			}
		}
	} else if(attrid == mSyncAtt){
		mNetPort = buf.read<int>();
		mBaseTime = buf.read<uint64_t>();

		mNetClock = buf.read<uint64_t>();
		mIpAddress = buf.read<std::string>();
		setNetClock();
	} 
	else if(attrid == mDoSyncAtt){
		mDoSyncronization = buf.read<bool>();
	}
	else if (attrid == mUpdateSeekTimeAtt){
		mSeekTime = buf.read<uint64_t>();
		mGstreamerWrapper->setSeekTime(mSeekTime);
	}
	else if (attrid == mUpdateBaseTimeAtt){
		mBaseTime = buf.read<uint64_t>();
		mGstreamerWrapper->setPipelineBaseTime(mBaseTime);
	}
	else if (attrid == mSeekAtt){
		mSeekTime = buf.read<uint64_t>();
		mGstreamerWrapper->seekFrame(mSeekTime);
	} 	 
	else if (attrid == mUpdateAtt){
		mBaseTime = buf.read<uint64_t>();
		mSeekTime = buf.read<uint64_t>();
		mGstreamerWrapper->setSeekTime(mSeekTime);
		mGstreamerWrapper->setPipelineBaseTime(mBaseTime);
		mGstreamerWrapper->setTimePositionInNs(mSeekTime);
	} 
	else if(attrid == mInstancesAtt){
		int numOfInstances = buf.read<int>();
		for(int i = 0; i < numOfInstances; i++){
			mPlayableInstances.push_back(buf.read<std::string>());
		}
	}
	else{
		Sprite::readAttributeFrom(attrid, buf);
	}
}
 
void GstVideo::writeClientAttributesTo(ds::DataBuffer& buf)const{
	// This means that we're a client that didn't actually load any video, so no need to write any data back to the server
	// I know it's confusing that clients can be in server-only mode, but here we are
	if(mServerOnlyMode || mStreaming){
		return;
	}

	ds::ScopedClientAtts scope(buf, getId());
	buf.add(mStatusAtt);
	buf.add(static_cast<float>(getCurrentPosition())); // position is really all we need, right?
}

void GstVideo::readClientAttributeFrom(const char attributeId, ds::DataBuffer& buf){
	// If we're getting stuff back from clients, then we probably know what it is
	// But make sure, cause anything else would be an error
	if(attributeId == mStatusAtt){
		const float clientPos = buf.read<float>();
		mServerPosition = clientPos;
	} else {
		DS_LOG_WARNING("Got an unexpected attribute back when reading client attributes. Probably a network packet error. Attribute=" << attributeId);
	}
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

void GstVideo::enablePlayingAFrame(bool on /*= true*/){
	if (mPlaySingleFrame == on) return;
	mPlaySingleFrame = on;
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

void GstVideo::setAllowOutOfBoundsMuted(const bool allowMuted) {
	mAllowOutOfBoundsMuted = allowMuted;
	applyMovieVolume();
}

GstVideo::Status::Status(int code){
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