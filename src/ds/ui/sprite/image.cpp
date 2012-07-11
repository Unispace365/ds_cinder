#include "image.h"
#include <map>
#include "cinder/ImageIo.h"

namespace {

std::map<std::string, std::shared_ptr<ci::gl::Texture>> mTextureCache;

}

namespace ds {
namespace ui {

Image::Image( SpriteEngine& engine, const std::string &filename )
  : inherited(engine)
{
    setTransparent(false);
    mTexture = getImage( filename );

    if ( !mTexture )
        return;

    Sprite::setSize(static_cast<float>(mTexture->getWidth()), static_cast<float>(mTexture->getHeight()));
}

Image::~Image()
{

}

void Image::drawLocal()
{
    if ( mTexture )
        ci::gl::draw(*mTexture);
}

void Image::setSize( float width, float height )
{
    setScale( width / getWidth(), height / getHeight() );
}

void Image::loadImage( const std::string &filename )
{
    mTexture = getImage( filename );
    float prevWidth = getWidth() * getScale().x;
    float prevHeight = getHeight() * getScale().y;

    if ( !mTexture )
        return;

    Sprite::setSize(static_cast<float>(mTexture->getWidth()), static_cast<float>(mTexture->getHeight()));
    setSize(prevWidth, prevHeight);
}

std::shared_ptr<ci::gl::Texture> Image::getImage( const std::string &filename )
{
    auto found = mTextureCache.find(filename);
    if ( found != mTextureCache.end() )
        return found->second;
    mTextureCache[filename] = std::move(std::shared_ptr<ci::gl::Texture>(new ci::gl::Texture));
    (*mTextureCache[filename]) = ci::loadImage( filename );
    return mTextureCache[filename];
}

} // namespace ui
} // namespace ds
