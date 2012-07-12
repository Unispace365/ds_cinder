#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H
#include "sprite.h"
#include <string>
#include "cinder/gl/Texture.h"

namespace ds {
namespace ui {

class Image: public Sprite
{
    public:
        Image( SpriteEngine&, const std::string &filename );
        ~Image();
        void                             setSize( float width, float height );
        void                             drawLocal();
        void                             loadImage( const std::string &filename );
    private:
        typedef Sprite inherited;

        std::shared_ptr<ci::gl::Texture> getImage( const std::string &filename );

        std::shared_ptr<ci::gl::Texture> mTexture;
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
