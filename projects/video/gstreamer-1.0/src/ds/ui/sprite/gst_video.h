#ifndef DS_UI_SPRITE_GST_VIDEO_H_
#define DS_UI_SPRITE_GST_VIDEO_H_

#include <cinder/gl/Texture.h>

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

	// Sets the video sprite size. Internally just scales the texture
	void				setSize( float width, float height );
	
protected:
	virtual void		updateClient(const UpdateParams&) override;
	virtual void		updateServer(const UpdateParams&) override;
	virtual void		drawLocalClient() override;
    void			    writeAttributesTo(DataBuffer&) override;
    void			    readAttributeFrom(const char, DataBuffer&) override;

public:
	// Loads a video from a file path.
	GstVideo&			loadVideo(const std::string &filename);
	// Loads a vodeo from a ds::Resource::Id
	GstVideo&			loadVideo(const ds::Resource::Id& resource_id);
	// Loads a vodeo from a ds::Resource
	GstVideo&			loadVideo(const ds::Resource& resource);

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

	struct Status
    {
        Status(int code);
        bool operator ==(int status) const;
        bool operator !=(int status) const;

		static const int  STATUS_STOPPED = 0;
		static const int  STATUS_PLAYING = 1;
		static const int  STATUS_PAUSED  = 2;
		
        int               mCode;
	};

    const Status&       getCurrentStatus() const;
    const std::string&  getLoadedVideoPath() const;

	// Callback when video changes its status (play / pause / stop).
	void				setStatusCallback(const std::function<void(const Status&)>&);

	// Sets the video complete callback. It's called when video is finished.
	void				setVideoCompleteCallback(const std::function<void()> &func);
	// Triggers the video complete callback. Ideally you will not need to use this but this is here
	// to give sufficient access to the Impl class without making it a 'friend' of GstVideo.
	void				triggerVideoCompleteCallback();

	// If a video is looping, will stop the video when the current loop completes.
	void				stopAfterNextLoop();
	
private:
	void				doLoadVideo(const std::string &filename);
	void				applyMovieVolume();
	void				applyMovieLooping();
    void                checkOutOfBounds();
    void                setStatus(const int);
    void                checkStatus();

private:
    typedef Sprite              inherited;
	std::shared_ptr<class Impl> mGstreamerWrapper;
	ci::gl::Texture             mFrameTexture;
	std::string                 mFilename;
	bool                        mLooping;
	bool                        mMuted;
	bool                        mAutoStart;
	bool                        mOutOfBoundsMuted;
	float                       mVolume;
	bool                        mShouldPlay;
	Status                      mStatus;
	bool                        mStatusChanged;
    std::function<void()>       mVideoCompleteFn;
	std::function<void(const Status&)>
                                mStatusFn;
};

} //!namespace ui
} //!namespace ds

#endif //!DS_UI_SPRITE_GST_VIDEO_H_