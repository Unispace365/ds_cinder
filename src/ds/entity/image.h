#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H
#include "entity.h"
#include <string>
#include "cinder/gl/Texture.h"

namespace ds
{

class Image: public Entity
{
    public:
        Image( const std::string &filename );
        ~Image();
        void                             setSize( float width, float height );
        void                             drawLocal();
        void                             loadImage( const std::string &filename );
    private:
        std::shared_ptr<ci::gl::Texture> getImage( const std::string &filename );

        std::shared_ptr<ci::gl::Texture> mTexture;
};

}

#endif//DS_IMAGE_H
