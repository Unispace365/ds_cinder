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
        Video( SpriteEngine&, const std::string &filename );
        ~Video();
        void                setSize( float width, float height );
        void                drawLocalClient();
        void                loadVideo( const std::string &filename );
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
    private:
        typedef Sprite inherited;

        ci::qtime::MovieGl  mMovie;
        ci::gl::Texture     mFrameTexture;
        ci::gl::Fbo         mFbo;

        bool                mLooping;
        bool                mMuted;
        float               mVolume;
};

} // namespace ui
} // namespace ds

#endif//DS_VIDEO_H
