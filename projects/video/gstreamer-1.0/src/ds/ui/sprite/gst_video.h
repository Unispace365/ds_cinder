#ifndef DS_UI_SPRITE_GST_VIDEO_H_
#define DS_UI_SPRITE_GST_VIDEO_H_

#include <cinder/gl/Texture.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>

#include <Poco/Timestamp.h>
//#include <gst/gstclock.h>

namespace gstwrapper {
	class GStreamerWrapper;
}

namespace ds {
namespace ui {

/**
 * \class ds::ui::GstVideo
 * \brief Video playback.
 * Note: If you want to use multiple playback systems simultaneously, use this
 * uniquely-named class. If you want to have one playback system that you can
 * swap out easily, include "video.h" and just use the Video class.
 */
class GstVideo : public Sprite
{
	/* Default starting port number for client/server video sync.  Used if not setting value provided.*/
const  int 					DEFAULT_PORT = 5637;

public:
	// Valid statuses for this player instance.
	struct Status {
		Status(int code);
		bool operator ==(int status) const;
		bool operator !=(int status) const;

		static const int	STATUS_STOPPED = 0;
		static const int	STATUS_PLAYING = 1;
		static const int	STATUS_PAUSED = 2;

		int					 mCode;
	};

public:

	// Convenience for allocating a Video sprite pointer and optionally adding it
	// to another Sprite as child.
	static GstVideo&	makeVideo(SpriteEngine&, Sprite* parent = nullptr);

	// Generic constuctor. To be used with Sprite::addChilePtr(...)
	GstVideo(SpriteEngine&);

	// Destructor, simply a no-op. Made virtual for polymorphisms.
	virtual ~GstVideo();

	// Sets the video sprite size. Internally just scales the texture
	void				setSize( float width, float height );
	
	// Loads a video from a file path.
	GstVideo&			loadVideo(const std::string &filename);
	// Loads a vodeo from a ds::Resource::Id
	GstVideo&			setResourceId(const ds::Resource::Id& resource_id);
	// Loads a vodeo from a ds::Resource
	GstVideo&			setResource(const ds::Resource& resource);

	// If clear frame is true then the current frame texture is removed. I
	// would think this should default to true but I'm maintaining compatibility
	// with existing behavior.
	void				unloadVideo(const bool clearFrame = false);

	void				startStream(const std::string& streamingPipeline, const float width, const float height);

	// Looping (play again after video complete)
	void				setLooping(const bool on);
	bool				getIsLooping() const;

	// Mutes the video
	void				setMute(const bool on);
	bool				getIsMuted() const;
	
	// Volume control. value between 0.0f and 1.0f
	void				setVolume(const float volume);
	float				getVolume() const;

	///Control which speaker to play out of (left/right/both) 
	///-1.0f == full left, 0.0f == center, 1.0f == right
	/// Note: you need to set generateAudioBuffer(true) AND not use a custom pipeline for this to work
	void				setPan(const float pan);
	float				getPan() const;

	// Playback control API 
	virtual void		play();
	virtual void		stop();
	virtual void		pause();
	bool				getIsPlaying() const;

	// Time operations (in seconds)
	double				getDuration() const;
	double				getCurrentTime() const;
	double				getCurrentTimeMs() const;
	void				seekTime(const double);

	// Position operations (in unit values, 0 - 1)
	double				getCurrentPosition() const;
	void				seekPosition(const double);

	// If true, will play the video as soon as it's loaded.
	void				setAutoStart(const bool doAutoStart);
	bool				getAutoStart() const;

	// Gets the current status of the player, in case you need if out of callback.
	const Status&		getCurrentStatus() const;
	
	// Gets the currently loaded filename (if any)
	const std::string&	getLoadedFilename() const;

	// Callback when video changes its status (play / pause / stop).
	void				setStatusCallback(const std::function<void(const Status&)>&);

	// Sets the video complete callback. It's called when video is finished.
	void				setVideoCompleteCallback(const std::function<void()> &func);

	// If a video is looping, will stop the video when the current loop completes.
	void				stopAfterNextLoop();

	// Play a single frame, then stop. Useful to show a thumbnail-like frame, or to keep a video in the background, but visible
	// Optional: Supply the time in ms to display
	// Optional: Supply a function called once that frame has been displayed (to unload the video, or animate or whatever)
	void				playAFrame(double time_ms = -1.0,const std::function<void()>& fn = nullptr);
	void				enablePlayingAFrame(bool on = true);
	bool				isPlayingAFrame() const;

	// Extends the idle timer for this sprite when the video is playing. Default = false
	void				setAutoExtendIdle(const bool doAutoextend);
	bool				getAutoExtendIdle() const;

	virtual void		updateClient(const UpdateParams&) override;
	virtual void		updateServer(const UpdateParams&) override;

	//Allow for custom audio output
	void				generateAudioBuffer(bool enableAudioBuffer);

	// In case you want a ton of info from gstreamer about what's going on
	void				setVerboseLogging(const bool doVerbose);

	// Calculates a rough fps for how many actual buffers we're displaying per second
	float				getVideoPlayingFramerate();

protected:
	virtual void		drawLocalClient() override;
	virtual void		writeAttributesTo(DataBuffer&) override;
	virtual void		readAttributeFrom(const char, DataBuffer&) override;
	gstwrapper::GStreamerWrapper* mGstreamerWrapper;

private:

	// A simple enum for specifying how the video gets rendered.
	// Transparent: retains an alpha channel through the whole pipeline, gstreamer handles the colorspace conversion to RGBA (or BGRA)
	// Solid: has no alpha channel, and gstreamer handles the colorspace conversion to RGB (or BGR)
	// ShaderTransform: has no alpha channel, and colorspace conversion is handled in a shader when drawing to the screen, uses I420 YUV colorspace conversion only
	// This value is assumed from the output of the videometa cache
	typedef enum { kColorTypeTransparent = 0, kColorTypeSolid, kColorTypeShaderTransform } ColorType;
	

	// filename is the absolute path to the file.
	// portable_filename is the CMS-relative path, so apps installed under different
	// user accounts and to different CMS locations can interoperate.
	void				doLoadVideo(const std::string &filename,
									const std::string &portable_filename);
	void				applyMovieVolume();
	void				applyMoviePan(const float pan);
	void				applyMovieLooping();
	void				checkOutOfBounds();
	void				setStatus(const int);
	void				checkStatus();
	void				setNetClock();

	ColorType			mColorType;

	ci::gl::Texture		mFrameTexture;
	ci::gl::Texture		mUFrameTexture;
	ci::gl::Texture		mVFrameTexture;

	ci::Vec2i			mVideoSize;
	double				mCachedDuration;
	std::string			mFilename,
						mPortableFilename;
	Status				mStatus;
	std::function<void()>
						mPlaySingleFrameFunction;
	std::function<void()>
						mVideoCompleteFn;
	std::function<void(const Status&)>
						mStatusFn;
	float				mVolume;
	float				mPan;
	bool				mLooping;
	bool				mMuted;
	bool				mOutOfBoundsMuted;
	bool				mAutoStart;
	bool				mShouldPlay;
	bool				mShouldSync;
	// There is actual video data ready to be drawn
	bool				mDrawable;
	// When the video is playing, extend the idle timer, or not
	bool				mAutoExtendIdle;
	// Playing a single frame, then stopping.
	bool				mPlaySingleFrame;
	bool				mStatusChanged;
	//Allow for custom audio output
	bool				mGenerateAudioBuffer;

	// YUV/I420 -> RGB conversion
	ci::gl::GlslProg	mShader;

	std::vector<Poco::Timestamp::TimeVal>	mBufferUpdateTimes;
	float									mCurrentGstFrameRate;

	std::uint64_t		mBaseTime;		//Base clock for gst pipeline
	std::uint64_t		mSeekTime;		//Position to seek to

	int					mNetPort;		// todo: increment per video sprite
	std::uint64_t		mNetClock;		// network clock time
	std::string			mIpAddress;		// Ip address of system clock for gstreamer pipeline to sync with


	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

} //!namespace ui
} //!namespace ds

#endif //!DS_UI_SPRITE_GST_VIDEO_H_