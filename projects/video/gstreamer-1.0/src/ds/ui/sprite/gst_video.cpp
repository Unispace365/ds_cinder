#include "gst_video.h"
#include "timer_sprite.h"
#include "gst_video_net.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/data/resource_list.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

#include "gstreamer/_2RealGStreamerWrapper.h"
#include "gstreamer/gstreamer_env_check.h"
#include "gstreamer/video_meta_cache.h"

#include <mutex>
#include "ds/app/engine/engine.h"

using namespace ci;
using namespace _2RealGStreamerWrapper;

namespace {
static ds::gstreamer::EnvCheck  ENV_CHECK;
ds::ui::VideoMetaCache          CACHE("gstreamer");
const ds::BitMask               GSTREAMER_LOG = ds::Logger::newModule("gstreamer");
template<typename T> void       noop(T) { /* no op */ };
void                            noop()  { /* no op */ };
} //!anonymous namespace

namespace ds {
namespace ui {

/*!
 * \class GstVideo::Impl
 * \namespace ds::ui
 * \brief Pimpl provider of GstVideo.
 * \note moved here to hide the forward declaration of GStreamerWrapper.
 */
class GstVideo::Impl {
public:
	Impl(GstVideo& holder)
		: mHolder(holder)
        , mTimer(nullptr)
		, mMoviePtr(std::make_unique<GStreamerWrapper>())
        , mNetHandler(holder)
	{}

	void handleVideoComplete() {
        mHolder.mVideoCompleteFn();
	}

	GStreamerWrapper& getMovieRef() {
		return *mMoviePtr;
	}

    TimerSprite& getTimerRef() {
        return *mTimer;
    }

    GstVideoNet& getNetHandler() {
        return mNetHandler;
    }

    void registerTimer(TimerSprite* const timer) {
        mTimer = timer;
    }

    void removeTimer() {
        if (hasTimer()) mTimer->release();
    }

    bool hasTimer() const {
        return mTimer != nullptr;
    }

	~Impl() {
		mMoviePtr->stop();
		mMoviePtr->close();
	}

private:
	GstVideo& mHolder;
    TimerSprite* mTimer;
    GstVideoNet mNetHandler;
	std::unique_ptr<GStreamerWrapper> mMoviePtr;
};

GstVideo& GstVideo::makeVideo(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::GstVideo>([&e]()->ds::ui::GstVideo*{ return new ds::ui::GstVideo(e); }, parent);
}

/**
 * \class ds::ui::sprite::Video static
 */
GstVideo::GstVideo(SpriteEngine& engine)
	: Sprite(engine)
	, mGstreamerWrapper(std::make_shared<Impl>(*this))
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
    , mSyncTolerance(35) // 35 ms
    , mStatus(Status::STATUS_STOPPED)
{
    mBlobType = GstVideoNet::mBlobType;

	setUseShaderTextuer(true);
    setTransparent(false);
}

GstVideo::~GstVideo() {}

void GstVideo::updateServer(const UpdateParams &up)
{
    Sprite::updateServer(up);
    
    checkStatus();
    checkOutOfBounds();

	mGstreamerWrapper->getMovieRef().update();
}

void GstVideo::updateClient(const UpdateParams& up)
{
    Sprite::updateClient(up);

    checkStatus();
    mGstreamerWrapper->getMovieRef().update();
}

void GstVideo::drawLocalClient()
{
    Sprite::drawLocalClient();

	if (mGstreamerWrapper->getMovieRef().hasVideo()
        && mGstreamerWrapper->getMovieRef().isNewVideoFrame())
    {
        ci::Surface video_surface(
            mGstreamerWrapper->getMovieRef().getVideo(),
            mGstreamerWrapper->getMovieRef().getWidth(),
            mGstreamerWrapper->getMovieRef().getHeight(),
            mGstreamerWrapper->getMovieRef().getWidth() * 4, // RGBA: therefore there is 4x8 bits per pixel, therefore 4 bytes per pixel.
            ci::SurfaceChannelOrder::RGBA);
		
        mFrameTexture.update(video_surface);
	}

	if (mFrameTexture)
    {
        ci::gl::draw(mFrameTexture);
	}
}

void GstVideo::onChildAdded(Sprite& child)
{
    if (!mGstreamerWrapper->hasTimer())
    {
        if (auto timer = dynamic_cast<TimerSprite*>(&child))
        {
            mGstreamerWrapper->registerTimer(timer);
            // every 10 frames is enough
            mGstreamerWrapper->getTimerRef().setTimerFrequency(5);
        }
    }

    Sprite::onChildAdded(child);
}

void GstVideo::setSize( float width, float height )
{
	setScale( width / getWidth(), height / getHeight() );
}

GstVideo& GstVideo::loadVideo(const std::string& filename)
{
    if (mFilename == filename)
    {
        return *this;
    }

	const std::string _filename(ds::Environment::expand(filename));

    if (_filename.empty())
    {
		DS_LOG_WARNING_M("GstVideo::loadVideo recieved a blank filename. Cancelling load.", GSTREAMER_LOG);
		return *this;
	}

	doLoadVideo(filename);
    markAsDirty(mGstreamerWrapper->getNetHandler().mPathDirty);
	return *this;
}

GstVideo &GstVideo::loadVideo(const ds::Resource::Id &resourceId)
{
	try
    {
		ds::Resource res;
		if (mEngine.getResources().get(resourceId, res))
        {
            loadVideo(res);
		}
	}
    catch (const std::exception& ex)
    {
		DS_LOG_WARNING_M("GstVideo::loadVideo() ex=" << ex.what(), GSTREAMER_LOG);
	}

	return *this;
}

GstVideo& GstVideo::loadVideo(const ds::Resource& resource)
{
    Sprite::setSizeAll(resource.getWidth(), resource.getHeight(), mDepth);
    loadVideo(resource.getAbsoluteFilePath());
    return *this;
}

void GstVideo::setLooping(const bool on)
{
	mLooping = on;
	applyMovieLooping();
    markAsDirty(mGstreamerWrapper->getNetHandler().mLoopingDirty);
}

bool GstVideo::getIsLooping() const
{
	return mLooping;
}
		
void GstVideo::setMute( const bool doMute )
{
	mMuted = doMute;
	applyMovieVolume();
    markAsDirty(mGstreamerWrapper->getNetHandler().mMuteDirty);
}

bool GstVideo::getIsMuted() const
{
	return mMuted;
}

void GstVideo::setVolume(const float volume)
{
	if (mVolume == volume) return;

	mVolume = volume;
	applyMovieVolume();

    markAsDirty(mGstreamerWrapper->getNetHandler().mVolumeDirty);
}

float GstVideo::getVolume() const
{
	return mVolume;
}

void GstVideo::play()
{
	mGstreamerWrapper->getMovieRef().play();

	//If movie not yet loaded, remember to play it later once it has
	if (!mGstreamerWrapper->getMovieRef().hasVideo())
    {
		mShouldPlay = true;
	}

    markAsDirty(mGstreamerWrapper->getNetHandler().mStatusDirty);
}

void GstVideo::stop()
{
	mShouldPlay = false;
	mGstreamerWrapper->getMovieRef().stop();
    markAsDirty(mGstreamerWrapper->getNetHandler().mStatusDirty);
}

void GstVideo::pause()
{
	mShouldPlay = false;
	mGstreamerWrapper->getMovieRef().pause();
    markAsDirty(mGstreamerWrapper->getNetHandler().mStatusDirty);
}

bool GstVideo::getIsPlaying() const
{
	if (mGstreamerWrapper->getMovieRef().getState() == PLAYING)
    {
		return true;
	}
    else
    {
		return false;
	}
}

double GstVideo::getDuration() const
{
	return mGstreamerWrapper->getMovieRef().getDurationInMs() / 1000.0;
}

double GstVideo::getCurrentTime() const
{
	return mGstreamerWrapper->getMovieRef().getPosition() * getDuration();
}
		
void GstVideo::seekTime(const double t)
{
	mGstreamerWrapper->getMovieRef().setTimePositionInMs(t * 1000.0);
}

double GstVideo::getCurrentPosition() const
{
	return mGstreamerWrapper->getMovieRef().getPosition();
}

void GstVideo::seekPosition(const double t)
{
	mGstreamerWrapper->getMovieRef().setPosition(t);
}

void GstVideo::setStatusCallback(const std::function<void(const Status&)>& fn)
{
	mStatusFn = fn;
}

void GstVideo::doLoadVideo(const std::string &filename)
{
	if (filename.empty()) return;
	
	VideoMetaCache::Type		type(VideoMetaCache::ERROR_TYPE);

	try
    {
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

		Sprite::setSizeAll(static_cast<float>(videoWidth), static_cast<float>(videoHeight), mDepth);

		if (getWidth() > 0 &&  getHeight() > 0)
        {
			setSize(getWidth() * getScale().x,  getHeight() * getScale().y);
		}

		DS_LOG_INFO_M("GstVideo::doLoadVideo() movieOpen", GSTREAMER_LOG);
		mGstreamerWrapper->getMovieRef().open(filename, generateVideoBuffer, false, true, videoWidth, videoHeight);

		applyMovieLooping();
		applyMovieVolume();

        mGstreamerWrapper->getMovieRef().setVideoCompleteCallback([this](GStreamerWrapper*){ mGstreamerWrapper->handleVideoComplete(); });

		setStatus(Status::STATUS_PLAYING);

	}
    catch (std::exception const& ex)
    {
		DS_LOG_ERROR_M("GstVideo::doLoadVideo() ex=" << ex.what(), GSTREAMER_LOG);
		return;
	}

	if (mGstreamerWrapper->getMovieRef().getWidth() < 1.0f || mGstreamerWrapper->getMovieRef().getHeight() < 1.0f)
    {
		if (type != VideoMetaCache::AUDIO_TYPE)
        {
			DS_LOG_WARNING_M("GstVideo::doLoadVideo() Video is too small to be used or didn't load correctly! "
                << filename
                << " "
                << getWidth()
                << " "
                << getHeight(), GSTREAMER_LOG);
		}
		return;
	}
    else
    {
        ci::gl::Texture::Format fmt;
        fmt.setInternalFormat(GL_RGBA);
        mFrameTexture = ci::gl::Texture(static_cast<int>(getWidth()), static_cast<int>(getHeight()), fmt);

        mFilename = filename;
    }
}

void GstVideo::setStatus(const int code)
{
	if (code == mStatus.mCode) return;

	mStatus.mCode = code;
	mStatusChanged = true;

    markAsDirty(mGstreamerWrapper->getNetHandler().mStatusDirty);
}

void GstVideo::applyMovieVolume()
{
	if (mMuted || mOutOfBoundsMuted)
    {
		mGstreamerWrapper->getMovieRef().setVolume(0.0f);
	}
    else
    {
		mGstreamerWrapper->getMovieRef().setVolume(mVolume);
	}
}

void GstVideo::applyMovieLooping()
{
	if (mLooping)
    {
		mGstreamerWrapper->getMovieRef().setLoopMode(LOOP);
	}
    else
    {
		mGstreamerWrapper->getMovieRef().setLoopMode(NO_LOOP);
	}
}

void GstVideo::unloadVideo(const bool clearFrame)
{
	mGstreamerWrapper->getMovieRef().stop();
	mGstreamerWrapper->getMovieRef().close();
	mFilename.clear();
	if (clearFrame)
    {
		mFrameTexture.reset();
	}
}

void GstVideo::setVideoCompleteCallback( const std::function<void()> &func )
{
	mVideoCompleteFn = func;
}

void GstVideo::setAutoStart( const bool doAutoStart )
{
	// do not check for mAutoStart == doAutoStart. There is no
	// correlation between them. mAutoStart is a cache value. Might be out of sync.
	mAutoStart = doAutoStart;
	mGstreamerWrapper->getMovieRef().setStartPlaying(mAutoStart);
}

bool GstVideo::getAutoStart() const
{
	return mAutoStart;
}

void GstVideo::stopAfterNextLoop()
{
	mGstreamerWrapper->getMovieRef().stopOnLoopComplete();
}

void GstVideo::checkStatus()
{
    if (mStatusChanged)
    {
        mStatusFn(mStatus);
        mStatusChanged = false;
    }

    if (mGstreamerWrapper->getMovieRef().getState() == STOPPED
        && mStatus != Status::STATUS_STOPPED)
    {
        setStatus(Status::STATUS_STOPPED);
    }
    else if (mGstreamerWrapper->getMovieRef().getState() == PLAYING
        && mStatus != Status::STATUS_PLAYING)
    {
        setStatus(Status::STATUS_PLAYING);
    }
    else if (mGstreamerWrapper->getMovieRef().getState() == PAUSED
        && mStatus != Status::STATUS_PAUSED)
    {
        setStatus(Status::STATUS_PAUSED);
    }
    
    if (mStatus != Status::STATUS_PLAYING
        && mGstreamerWrapper->getMovieRef().hasVideo() && mShouldPlay)
    {
        play();
        mShouldPlay = false;
    }
}

void GstVideo::checkOutOfBounds()
{
    if (!inBounds())
    {
        if (!mOutOfBoundsMuted)
        {
            mGstreamerWrapper->getMovieRef().setVolume(0.0f);
            mOutOfBoundsMuted = true;
        }
        return;
    }
    else if (mOutOfBoundsMuted)
    {
        mOutOfBoundsMuted = false;
        applyMovieVolume();
    }
}

const GstVideo::Status& GstVideo::getCurrentStatus() const
{
    return mStatus;
}

const std::string& GstVideo::getLoadedFilename() const
{
    return mFilename;
}

void GstVideo::writeAttributesTo(DataBuffer& buf)
{
    Sprite::writeAttributesTo(buf);
    mGstreamerWrapper->getNetHandler().writeAttributesTo(mDirty, buf);
}

void GstVideo::readAttributeFrom(const char id, DataBuffer& buf)
{
    if (!mGstreamerWrapper->getNetHandler().readAttributeFrom(id, buf))
        Sprite::readAttributeFrom(id, buf);
}

void GstVideo::enableSynchronization(bool on /*= true*/)
{
    if (mShouldSync == on) return;
    mShouldSync = on;

    if (mShouldSync)
    {
        static std::once_flag _call_once;
        std::call_once(_call_once, [this]{
            mGstreamerWrapper->registerTimer(addChildPtr(new ds::ui::TimerSprite(mEngine)));
            mGstreamerWrapper->getTimerRef().setTimerCallback([this]{
                markAsDirty(mGstreamerWrapper->getNetHandler().mPosDirty);
            });
        });
    }
    else
    {
        mGstreamerWrapper->removeTimer();
    }
}

void GstVideo::syncWithServer(double server_time)
{
    if (mGstreamerWrapper->hasTimer())
    {
        auto server_diff =
            mGstreamerWrapper->getTimerRef().now()
            - mGstreamerWrapper->getTimerRef().getServerSendTime();

        auto my_expected_time =
            server_time
            + mGstreamerWrapper->getTimerRef().getLatency();

        auto time_diff = ci::math<double>::abs(getCurrentTimeMs() - my_expected_time);

        if (time_diff > mSyncTolerance)
        {
            mGstreamerWrapper->getMovieRef().setTimePositionInMs(my_expected_time + server_diff);
        }
    }
}

double GstVideo::getCurrentTimeMs() const
{
    return mGstreamerWrapper->getMovieRef().getCurrentTimeInMs();
}

void GstVideo::setSyncTolerance(double time_ms)
{
    if (mSyncTolerance == time_ms) return;
    mSyncTolerance = time_ms;
}

GstVideo::Status::Status(int code)
{
    mCode = code;
}

bool GstVideo::Status::operator==(int status) const
{
    return mCode == status;
}

bool GstVideo::Status::operator!=(int status) const
{
    return mCode != status;
}

} //!namespace ui
} //!namespace ds