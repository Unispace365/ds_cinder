#pragma once
#ifndef DS_VIDEO_H
#define DS_VIDEO_H
#include "sprite.h"
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/gl/Fbo.h"

namespace ds {
namespace ui {

class Video: public Sprite
{
    public:
        Video( SpriteEngine& );
        ~Video();
        void                setSize( float width, float height );
        virtual void        updateServer(const UpdateParams&);
        void                drawLocalClient();
        Video&              loadVideo( const std::string &filename );
        void                play();
        void                stop();
        void                pause();
        void                seek(float t);
        float               duration() const;
        bool                isPlaying() const;
        void                loop(bool flag);
        bool                isLooping() const;
        // value between 0.0f & 1.0f
        void                setVolume(float volume);
        float               getVolume() const;

        struct Status {
          static const int  STATUS_STOPPED = 0;
          static const int  STATUS_PLAYING = 1;
          int               mCode;
        };
        void                setStatusCallback(const std::function<void(const Status&)>&);

    private:
        typedef Sprite inherited;

        void                setStatus(const int);

        ci::qtime::MovieGl  mMovie;
        ci::gl::Texture     mFrameTexture;
        ci::gl::Fbo         mFbo;

        bool                mLooping;
        bool                mMuted;
        float               mVolume;

        Status              mStatus;
        bool                mStatusDirty;
        std::function<void(const Status&)>
                            mStatusFn;
};

} // namespace ui
} // namespace ds

#endif//DS_VIDEO_H
