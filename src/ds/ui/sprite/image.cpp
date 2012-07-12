#include "image.h"

#include <map>
#include <cinder/ImageIo.h>
#include "ds/ui/sprite/sprite_engine.h"

namespace {

std::map<std::string, std::shared_ptr<ci::gl::Texture>> mTextureCache;

}

namespace ds {
namespace ui {

Image::Image( SpriteEngine& engine, const std::string &filename )
    : inherited(engine)
    , mImageService(engine.getLoadImageService())
    , mImageToken(mImageService)
    , mFlags(0)
    , mResourceFn(filename)
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

  float				      fade = 1;
  ci::gl::Texture	  *img = mImageToken.getImage(fade);

  // XXX Do bounds check here
  if (!img && mImageToken.canAcquire()) { // && intersectsLocalScreen){
    acquireImage();
    // Take one more pass, in case the image is currently being cached in the service.
    img = mImageToken.getImage(fade);
  }

  if (img) {
//    std::cout << "draw LOADED image!!!" << std::endl;
//    ci::gl::draw(*img);
  }
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

void Image::acquireImage()
{
  // XXX Check to see if I have a resource ID, and use that instead.
  if (mResourceFn.empty()) return;

  mImageToken.acquire(mResourceFn, mFlags);
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
