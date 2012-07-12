#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H
#include "sprite.h"
#include <string>
#include <cinder/gl/Texture.h>
#include "ds/ui/service/load_image_service.h"

namespace ds {
namespace ui {
class LoadImageService;

class Image: public Sprite
{
    public:
        // Cache any texture loaded by this sprite, never releasing it.
        static const int                IMG_CACHE_F = (1<<0);

    public:
        Image( SpriteEngine&, const std::string &filename );
        ~Image();
        void                            setSize( float width, float height );
        void                            drawLocal();
        void                            loadImage( const std::string &filename );
    private:
        typedef Sprite inherited;

        LoadImageService&               mImageService;
        ImageToken                      mImageToken;
        ci::gl::Texture                 mTexture;

        int                             mFlags;
        // The resource can either be a Resource::Id or a filename, but in all
        // cases it gets resolved to a file before loading
        std::string                     mResourceFn;

        void                            requestImage();
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
