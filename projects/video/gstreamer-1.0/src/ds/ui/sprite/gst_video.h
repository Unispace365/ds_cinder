#ifndef DS_UI_SPRITE_GST_VIDEO_H_
#define DS_UI_SPRITE_GST_VIDEO_H_

#include <cinder/gl/Texture.h>
#include <cinder/gl/Fbo.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/data/resource.h>

namespace ds {
namespace ui {

/**
 * \class ds::ui::GstVideo
 * \brief Video playback.
 * Note: If you want to use multiple playback systems simultaneously, use this
 * uniquely-named class. If you want to have one playback system that you can
 * swap out easily, include "video.h" and just use the Video class.
 */
class GstVideo : public Sprite {
public:

	// Convenience for allocating a Video sprite pointer and optionally adding it
	// to another Sprite as child.
	static GstVideo&	makeVideo(SpriteEngine&, Sprite* parent = nullptr);

	// Generic constuctor. To be used with Sprite::addChilePtr(...)
	GstVideo(SpriteEngine&);

	// Destructor, simply a no-op. Made virtual for polymorphisms.
	virtual ~GstVideo();

	// Sets the video alpha mode. If transparent, video texture will be RGBA
	// \note Set this before loading a video
	void				setAlphaMode(bool isTransparent);

	// Sets the video sprite size. Internally just scales the texture
	void				setSize( float width, float height );
	
protected:
	virtual void		updateClient(const UpdateParams&) override;
	virtual void		updateServer(const UpdateParams&) override;
	virtual void		drawLocalClient() override;

public:
	// Loads a video from a file path.
	GstVideo&			loadVideo(const std::string &filename);
	// Loads a vodeo from a ds::Resource::Id
	GstVideo&			setResourceId(const ds::Resource::Id&);

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
	void				seekTime(const double);

    // Position operations (in unit values, 0 - 1)
	double				getCurrentPosition() const;
	void				seekPosition(const double);

	// If true, will play the video as soon as it's loaded.
	void				setAutoStart(const bool doAutoStart);
	bool				getAutoStart() const;

	struct Status {
		static const int  STATUS_STOPPED = 0;
		static const int  STATUS_PLAYING = 1;
		static const int  STATUS_PAUSED  = 2;
		int               mCode;
	};

	// Callback when video changes its status (play / pause / stop).
	void				setStatusCallback(const std::function<void(const Status&)>&);

	// Sets the video complete callback. It's called when video is finished.
	void				setVideoCompleteCallback(const std::function<void(GstVideo* video)> &func);
	// Triggers the video complete callback. Ideally you will not need to use this but this is here
	// to give sufficient access to the Impl class without making it a 'friend' of GstVideo.
	void				triggerVideoCompleteCallback();

	// Set's the video to play, then stops the video after that frame has played.
	// Optionally supply a function called once I've played a frame.
	void				playAFrame(const std::function<void(GstVideo&)>& fn = nullptr);
	bool				isPlayingAFrame() const;
	// If a video is looping, will stop the video when the current loop completes.
	void				stopAfterNextLoop();
	
	// Total hack as I begin to work in the UDP replication. When a video sprite
	// is in server mode, it will never load or play a video, instead it becomes
	// a passthrough to a video running somewhere on a client machine, reporting
	// its play position etc. This would go away if we had fully sync'd, bounds-
	// checked video.
	void				setServerModeHack(const bool);
	// This is a hack because it only does the check once, not continuously,
	// so any sprites that move into or out of bound won't work.
	// NOTE: Needs to be set BEFORE loadVideo(), that's where the check is
	// occurring right now.
	void				setCheckBoundsHack(const bool = false);

protected:
	virtual void		writeAttributesTo(ds::DataBuffer&) override;
	virtual void		writeClientAttributesTo(ds::DataBuffer&) const override;
	virtual void		readAttributeFrom(const char attributeId, ds::DataBuffer&) override;
	virtual void		readClientAttributeFrom(const char attributeId, ds::DataBuffer&) override;

private:
	typedef Sprite		inherited;

	void				doLoadVideoMeta(const std::string &filename);
	void				doLoadVideo(const std::string &filename);
	void				onSetFilename(const std::string&);
	void                setStatus(const int);
	void				setMovieVolume();
	void				setMovieLooping();
	void				setVideoFlag(const uint32_t, const bool on);

private:
	// Done this way so I can completely hide any dependencies (even fwd decl's)
	std::shared_ptr<class Impl>	mPimpl;
	ci::gl::Texture     mFrameTexture;
	ci::gl::Fbo         mFbo;

	std::string			mFilename;
	bool				mFilenameChanged;
	bool                mLooping;
	// User-driven mute state
	bool				mMuted;
	// Cached value of autoStart (wrapper does not supply a getter)
	bool				mAutoStart;
	// A mute state that gets turned on automatically in certain situations
	bool                mInternalMuted;
	float               mVolume;
	bool				mIsTransparent;
	enum Cmd			{ kCmdPlay, kCmdPause, kCmdStop };
	Cmd					mCmd;
	void				setCmd(const Cmd);
	bool				mDoPlay; //remember to play file if it hasn't loaded yet

	Status              mStatus;
	bool                mStatusDirty;

	bool				mPlaySingleFrame;
	std::function<void(GstVideo&)>
						mPlaySingleFrameFn;

	std::function<void(const Status&)>
						mStatusFn;
	std::function<void(GstVideo*)>
						mVideoCompleteCallback;

	// Total hack as I begin to work in the UDP replication. When a video sprite
	// is in server mode, it will never load or play a video, instead it becomes
	// a passthrough to a video running somewhere on a client machine, reporting
	// its play position etc.
	bool				mServerModeHack;
	double				mReportedCurrentPosition;
	uint32_t			mVideoFlags;

	// Initialization
public:
	static void			installAsServer(ds::BlobRegistry&);
	static void			installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_GST_VIDEO_H_
