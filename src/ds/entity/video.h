#pragma once
#ifndef DS_VIDEO_H
#define DS_VIDEO_H
#include "entity.h"
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"

namespace ds
{

class Video: public Entity
{
    public:
        Video( const std::string &filename );
        ~Video();
        void                setSize( float width, float height );
        void                drawLocal();
        void                loadVideo( const std::string &filename );
        void                play();
        void                stop();
        void                pause();
        void                seek(float t);
        float               duration() const;
        bool                isPlaying() const;
        void                loop(bool flag);
        bool                isLooping() const;
    private:
        ci::qtime::MovieGl  mMovie;
        ci::gl::Texture     mFrameTexture;

        bool                mLooping;
};

}

#endif//DS_VIDEO_H
