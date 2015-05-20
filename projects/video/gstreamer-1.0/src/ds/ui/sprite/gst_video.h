#ifndef DS_UI_SPRITE_GST_VIDEO_H_
#define DS_UI_SPRITE_GST_VIDEO_H_

#include <cinder/gl/Texture.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>
#include "gst_video_net.h"

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
public:
	// Valid statuses for this player instance.
	struct Status
	{
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

	// Looping (play again after video complete)
	void				setLooping(const bool on);
	bool				getIsLooping() const;

	// Mutes the video
	void				setMute(const bool on);
	bool				getIsMuted() const;
	
	// Volume control. value between 0.0f and 1.0f
	void				setVolume(const float volume);
	float				getVolume() const;

	// Playback control API
	void				play();
	void				stop();
	void				pause();
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
	bool				isPlayingAFrame() const;

	// Extends the idle timer for this sprite when the video is playing. Default = false
	void				setAutoExtendIdle(const bool doAutoextend);
	bool				getAutoExtendIdle() const;

	virtual void		updateClient(const UpdateParams&) override;
	virtual void		updateServer(const UpdateParams&) override;

protected:
	virtual void		drawLocalClient() override;
	virtual void		writeAttributesTo(DataBuffer&) override;
	virtual void		readAttributeFrom(const char, DataBuffer&) override;

private:
	void				doLoadVideo(const std::string &filename);
	void				applyMovieVolume();
	void				applyMovieLooping();
	void				checkOutOfBounds();
	void				setStatus(const int);
	void				checkStatus();

private:

	GstVideoNet							mNetHandler;
	gstwrapper::GStreamerWrapper*		mGstreamerWrapper;

	ci::gl::Texture						mFrameTexture;
	ci::Vec2i							mVideoSize;
	std::string							mFilename;

	bool								mLooping;

	bool								mMuted;
	bool								mOutOfBoundsMuted;
	float								mVolume;

	bool								mAutoStart;
	bool								mShouldPlay;
	bool								mShouldSync;

	// There is actual video data ready to be drawn
	bool								mDrawable;

	// When the video is playing, extend the idle timer, or not
	bool								mAutoExtendIdle;
	
	// Playing a single frame, then stopping.
	bool								mPlaySingleFrame;
	std::function<void()>				mPlaySingleFrameFunction;

	Status								mStatus;
	bool								mStatusChanged;
	std::function<void()>				mVideoCompleteFn;
	std::function<void(const Status&)>	mStatusFn;
};

} //!namespace ui
} //!namespace ds

#endif //!DS_UI_SPRITE_GST_VIDEO_H_