#pragma once
#ifndef DS_VIDEO_H
#define DS_VIDEO_H
#include "sprite.h"
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/gl/Fbo.h"
#include "ds/data/resource.h"

namespace ds {
namespace ui {

class Video: public Sprite
{
    public:
        Video( SpriteEngine& );
        ~Video();

		void				clear();

        void                setSize( float width, float height );
        virtual void        updateServer(const UpdateParams&);
        void                drawLocalClient();
        Video&              loadVideo( const std::string &filename );
        Video              &setResourceId(const ds::Resource::Id &resourceId);
        void                play();
        void                stop();
        void                pause();
        void                seek(float t);
        float               duration() const;
        float               currentTime() const;
        bool                isPlaying() const;
        void                loop(bool flag);
        bool                isLooping() const;
        // value between 0.0f & 1.0f
        void                setVolume(float volume, const bool turnOffMute = true);
        float               getVolume() const;
		void				setMute(const bool on);
		bool				getMute() const;

        struct Status {
          static const int  STATUS_STOPPED = 0;
          static const int  STATUS_PLAYING = 1;
          static const int  STATUS_PAUSED  = 2;
          int               mCode;
        };
        void                setStatusCallback(const std::function<void(const Status&)>&);

    private:
        typedef Sprite inherited;

        void                setStatus(const int);
		void				setMovieVolume();

        ci::qtime::MovieGl  mMovie;
//        ci::gl::Texture     mFrameTexture;
        ci::gl::Fbo         mFbo;

        bool                mLooping;
        float               mVolume;
		bool				mMute;
		// Not entirely sure but looks like jeremy is using this
		// to control volume around playback starts.
        bool                mInternalMuted;

        Status              mStatus;
        bool                mStatusDirty;
        std::function<void(const Status&)>
                            mStatusFn;
};

} // namespace ui
} // namespace ds

#endif//DS_VIDEO_H
