#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H
#include "sprite.h"
#include <string>
#include <cinder/gl/Texture.h>
#include "ds/data/resource.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {
namespace ui {
class LoadImageService;

class Image: public Sprite
{
    public:
        static void                     install(ds::BlobRegistry&);

        // Cache any texture loaded by this sprite, never releasing it.
        static const int                IMG_CACHE_F = (1<<0);

    public:
        Image( SpriteEngine& );
        Image( SpriteEngine&, const std::string &filename );
        Image( SpriteEngine&, const ds::Resource::Id &resourceId );
        ~Image();

        void                            setSize( float width, float height );
        void                            drawLocalClient();
        void                            loadImage( const std::string &filename );
        bool                            isLoaded() const;
    private:
        typedef Sprite inherited;

        LoadImageService&               mImageService;
        ImageToken                      mImageToken;
        ci::gl::Texture                 mTexture;

        int                             mFlags;
        ds::Resource::Id                mResourceId;
        std::string                     mResourceFn;

        void                            requestImage();
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
